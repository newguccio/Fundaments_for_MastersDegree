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
//biblioteki lcd.h nie trzeba bo jest w pliku hagl_color
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern int16_t pos_x = 0;
extern int16_t pos_y = 0;
//nw czy trzeba ale narazie zostawiam
extern const unsigned char font6x9[];
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

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

void StartDefaultTask(void *argument);
void Break_warning(void *argument);
void adj_position(void *argument);

void MX_FREERTOS_Init(void *pdisplay); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void *pdisplay) {
  /* USER CODE BEGIN Init */

	//bierzemy adres tego z maiina i tutaj lokalnie tworzymy i zmieniamy strukture
	hagl_backend_t* backend = (hagl_backend_t*)pdisplay;
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
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
  defaultTaskHandle = osThreadNew(StartDefaultTask, backend, &defaultTask_attributes);

  /* creation of ir_sensor */
  ir_sensorHandle = osThreadNew(Break_warning, backend, &ir_sensor_attributes);

  /* creation of open_sensor */
  open_sensorHandle = osThreadNew(Break_warning, backend, &open_sensor_attributes);

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
	hagl_backend_t* backend = (hagl_backend_t*)argument;
	uint8_t value;
	static uint8_t last_value = 0; // musi byc wspolne dla kazdego wykonania
	//wchar_t text_buffer[20];
  /* USER CODE BEGIN Break_warning */
  /* Infinite loop */
  for(;;)
  {
	  hagl_clear(backend);
	  value = HAL_GPIO_ReadPin(door_sensor_GPIO_Port, door_sensor_Pin);
	  if(value != last_value){
		  last_value = value;
		  hagl_put_text(backend, L"otwarte drzwi", 0, 0, BLUE, font6x9);
		  lcd_copy();
	  }

      osDelay(100);
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
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END adj_position */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

