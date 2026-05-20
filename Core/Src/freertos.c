/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
		//ja dodalem
//#include "iks4a1_motion_sensors_ex.h"		//ja dodalem
//biblioteki lcd.h nie trzeba bo jest w pliku hagl_color
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern int16_t pos_x; // nie moge dac =0; bo to dziala jakbym inicjalizowal a tego nie robie, ja tylko pokazuje ze to jest
extern int16_t pos_y;
//nw czy trzeba ale narazie zostawiam
extern const unsigned char font6x9[];

typedef struct{

	hagl_backend_t* backend;
	GPIO_TypeDef* port;
	uint16_t pin;
	wchar_t* msg;
	uint16_t color;
	uint16_t last_value;
	uint16_t value;
}SensorConfig_h;


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

//handle do adresu mutexu
osMutexId_t screen_mutexHandle;

osMutexAttr_t screen_mutexAttribute ={
		"screenMutex",
		osMutexPrioInherit | osMutexRecursive,  //dziedziczenie priorytetu i uniemozliwienie zablokowanie taskowi samego siebie jesli kilka razy pod rzad jest ten sam
		NULL,
		0U
};

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ir_sensor */
osThreadId_t ir_sensorHandle;
const osThreadAttr_t ir_sensor_attributes = {
  .name = "ir_sensor",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for open_sensor */
osThreadId_t open_sensorHandle;
const osThreadAttr_t open_sensor_attributes = {
  .name = "open_sensor",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal7,
};
/* Definitions for ship_position */
osThreadId_t ship_positionHandle;
const osThreadAttr_t ship_position_attributes = {
  .name = "ship_position",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for safety_queue */
osMessageQueueId_t safety_queueHandle;
const osMessageQueueAttr_t safety_queue_attributes = {
  .name = "safety_queue"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument); // zawsze musimy przekazywac taskowi void a wewntarz mozemy przypisac cokolwiek juz
void Break_warning(void *argument);
void adj_position(void *argument);

void MX_FREERTOS_Init(void *pdisplay); /* (MISRA C 2004 rule 8.1) */

void lcd_use(hagl_backend_t* backend, const wchar_t* message, uint16_t color ); //const tak zapobiegawczo
/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void *pdisplay) {
  /* USER CODE BEGIN Init */

	//bierzemy adres tego z maiina i tutaj lokalnie tworzymy i zmieniamy strukture
	hagl_backend_t* backend = (hagl_backend_t*)pdisplay;

	static SensorConfig_h DoorSensor; //zeby byla przez caly czas dzialania programu
	static SensorConfig_h IRSensor;

	DoorSensor.backend = backend;
	DoorSensor.msg = L"Drzwi otwarte";
	DoorSensor.last_value = 0;
	DoorSensor.value = 1;
	DoorSensor.pin = door_sensor_Pin;
	DoorSensor.port = door_sensor_GPIO_Port;
	DoorSensor.color = BLUE;


	IRSensor.backend = backend;
	IRSensor.msg = L"wykryto ruch";
	IRSensor.last_value = 0;
	IRSensor.value = 1;
	IRSensor.pin = ir_sensor_Pin;
	IRSensor.port = ir_sensor_GPIO_Port;
	IRSensor.color = GREEN;
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */

	//mutex create
	screen_mutexHandle = osMutexNew(&screen_mutexAttribute);

  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of safety_queue */
  safety_queueHandle = osMessageQueueNew (6, sizeof(uint16_t), &safety_queue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, backend, &defaultTask_attributes); //zawsze przekazujemy jako argumen &zmienna albo i nie

  /* creation of ir_sensor */
  ir_sensorHandle = osThreadNew(Break_warning, &IRSensor, &ir_sensor_attributes);

  /* creation of open_sensor */
  open_sensorHandle = osThreadNew(Break_warning, &DoorSensor, &open_sensor_attributes);

  /* creation of ship_position */
  ship_positionHandle = osThreadNew(adj_position, backend, &ship_position_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_Break_warning */
/**
* @brief Function implementing the ir_sensor thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Break_warning */



void Break_warning(void *argument)
{
	SensorConfig_h* sensor = (SensorConfig_h*)argument;

  /* USER CODE BEGIN Break_warning */
	uint8_t value = sensor->value;
  /* Infinite loop */
  for(;;)
  {
	  value = HAL_GPIO_ReadPin(sensor->port, sensor->pin);
	  if(value != sensor->last_value){

		  if(value == sensor->value) lcd_use(sensor->backend, sensor->msg , sensor->color); // musi byc L przed bo dlugi char 16bitowy

		  sensor->last_value = value;
	  }
      osDelay(50);
  }
  /* USER CODE END Break_warning */
}

/* USER CODE BEGIN Header_adj_position */
/**
* @brief Function implementing the ship_position thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_adj_position */
void adj_position(void *argument)
{
  /* USER CODE BEGIN adj_position */

	hagl_backend_t* backend = (hagl_backend_t*)argument;

	IKS4A1_MOTION_SENSOR_AxesRaw_t axes;

	wchar_t axes_buffer[64]= {0}; // bo wchar oczekuje lcd_use
  /* Infinite loop */
  for(;;)
  {
// !!!!cos pierdzieli ta bilbioteka i jest multiple definition dla malloca, narazie dalem allow multiple definition ale to gowno wiec do zmiany !!!!

	  memset(axes_buffer, 0, sizeof(axes_buffer));
	  IKS4A1_MOTION_SENSOR_GetAxesRaw(IKS4A1_LSM6DSV16X_0, MOTION_GYRO, &axes);
	 //printf("GYRO DATA: %d %d %d \n", (int)axes.x, (int)axes.y, (int)axes.z);


	//  swprintf(axes_buffer, 64, L"GYR X%dY%dZ%d", (int)axes.x, (int)axes.y, (int)axes.z);
	 // lcd_use(backend, axes_buffer , RED);//typecasting tak na wszelki

	  memset(axes_buffer, 0, sizeof(axes_buffer));
	  IKS4A1_MOTION_SENSOR_GetAxesRaw(IKS4A1_LSM6DSV16X_0, MOTION_ACCELERO, &axes);
	  //printf("ACCELERO DATA: %d %d %d \n", (int)axes.x, (int)axes.y, (int)axes.z);
	//  swprintf(axes_buffer, 64, L"ACC X%dY%dZ%d", (int)axes.x, (int)axes.y, (int)axes.z);
	 // lcd_use(backend, axes_buffer , WHITE);

    osDelay(400);
  }
  /* USER CODE END adj_position */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void lcd_use(hagl_backend_t* backend, const wchar_t* message, uint16_t color ){

	  if(osMutexAcquire(screen_mutexHandle, osWaitForever) == osOK){

		hagl_put_text(backend, message, pos_x, pos_y, color, font6x9);
		pos_y += 12;
		hagl_fill_rectangle(backend, 0, pos_y, 160, pos_y + 12, BLACK);

		  		if(pos_y > 128) {
		  			pos_y  = 0;
		  			pos_x  = 0;
		  		}
		  		lcd_copy();

	  osMutexRelease(screen_mutexHandle);
	  }
}


/* USER CODE END Application */

