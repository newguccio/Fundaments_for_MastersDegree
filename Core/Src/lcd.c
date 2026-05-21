#include <stdio.h>

#include "lcd.h"
#include "spi.h"


// potrzbne zeby ekran wiedzial o rtosie i mozna bylo zawiesci taski
#include "FreeRTOS.h"
#include "task.h"

//makra z datasheetu ekranu ktore odpowiadaja za komunikacje z wyswietlaczem
#define ST7735S_SLPOUT			0x11
#define ST7735S_DISPOFF			0x28
#define ST7735S_DISPON			0x29
#define ST7735S_CASET			0x2a		//ustawianie kolumny
#define ST7735S_RASET			0x2b		//ustawianie wiersza
#define ST7735S_RAMWR			0x2c
#define ST7735S_MADCTL			0x36
#define ST7735S_COLMOD			0x3a
#define ST7735S_FRMCTR1			0xb1
#define ST7735S_FRMCTR2			0xb2
#define ST7735S_FRMCTR3			0xb3
#define ST7735S_INVCTR			0xb4
#define ST7735S_PWCTR1			0xc0
#define ST7735S_PWCTR2			0xc1
#define ST7735S_PWCTR3			0xc2
#define ST7735S_PWCTR4			0xc3
#define ST7735S_PWCTR5			0xc4
#define ST7735S_VMCTR1			0xc5
#define ST7735S_GAMCTRP1		0xe0
#define ST7735S_GAMCTRN1		0xe1

#define CMD(x)					((x) | 0x100) //	piszemy CMD(jedno z makr z góry) -> i teraz juz automatycznie

//static bo uzywamy ich tylko w tym pliku do kolejnych funkcji ktore dopiero sa przekazywane uzytkownikow- zapewnia to prywatnosc
static void lcd_command(uint8_t command){


	//vTaskSuspendAll(); //v bo zwraca nic


	HAL_GPIO_WritePin(lcd_dc_GPIO_Port, lcd_dc_Pin, 0); // zamiast 0 mozna dac GPIO_PIN_RESET ale dla mnie tak czytelniej jest jak jest tu
	HAL_GPIO_WritePin(lcd_cs_GPIO_Port, lcd_cs_Pin, 0);	//
	HAL_SPI_Transmit(&hspi1, &command, 1, 1);			//adres magistrali SPI ktorej chcemy uzyc, adres komendy, wartosc, czas- jesli cos sie jebie to te 10 moze rozwalac RTOS, ale dajemy hal_max_delay
	HAL_GPIO_WritePin(lcd_cs_GPIO_Port, lcd_cs_Pin, 1);


	//xTaskResumeAll(); //x bo zwraca flage ktora inny program musi dostac jezeli ma wznowic dzialanie

}

static void lcd_data(uint8_t data){


	//vTaskSuspendAll(); //v bo zwraca nic


	HAL_GPIO_WritePin(lcd_dc_GPIO_Port, lcd_dc_Pin, 1); // zamiast 0 mozna dac GPIO_PIN_RESET ale dla mnie tak czytelniej jest jak jest tu
	HAL_GPIO_WritePin(lcd_cs_GPIO_Port, lcd_cs_Pin, 0);	//
	HAL_SPI_Transmit(&hspi1, &data, 1, 1);
	HAL_GPIO_WritePin(lcd_cs_GPIO_Port, lcd_cs_Pin, 1);


	//xTaskResumeAll(); //x bo zwraca flage ktora inny program musi dostac jezeli ma wznowic dzialanie

}

static void lcd_send(uint16_t value){ 	//bierze wartosc i sprawdza czy jest 1 na 9 pozycji i w zalezsnoci od tego przechodzi do funkcji od komend albo danych

	if(value >= 256){					// if(jesli jest 1 w tym miejscu == true), czyli nakladasz maske i jesli byla tam jedynka to zwraca wlasnie true i sie wykona to co jest pod tym
		lcd_command(value);				//jesli jest 1 to ==command wiec wysylam jako komende
	}else{
		lcd_data(value);
	}

}


//potrzbne tylko do ustawienia warunkow pracy ekranu
static const uint16_t init_table[] = {			//wywołujemy komendy a potem wpisujemy dla niej dane
  CMD(ST7735S_FRMCTR1), 0x01, 0x2c, 0x2d,  		//wywołujemy komende --> wysylaj dane potrzebne do ustawienia podczas tej komendy
  CMD(ST7735S_FRMCTR2), 0x01, 0x2c, 0x2d,		// 1 krok:dla ST7735S_FRMCTR2 ustaw ze to koemnda 2 krok: a pozostałe to dane dla tej komendy
  CMD(ST7735S_FRMCTR3), 0x01, 0x2c, 0x2d, 0x01, 0x2c, 0x2d,
  CMD(ST7735S_INVCTR), 0x07,
  CMD(ST7735S_PWCTR1), 0xa2, 0x02, 0x84,
  CMD(ST7735S_PWCTR2), 0xc5,
  CMD(ST7735S_PWCTR3), 0x0a, 0x00,
  CMD(ST7735S_PWCTR4), 0x8a, 0x2a,
  CMD(ST7735S_PWCTR5), 0x8a, 0xee,
  CMD(ST7735S_VMCTR1), 0x0e,
  CMD(ST7735S_GAMCTRP1), 0x0f, 0x1a, 0x0f, 0x18, 0x2f, 0x28, 0x20, 0x22,
                         0x1f, 0x1b, 0x23, 0x37, 0x00, 0x07, 0x02, 0x10,
  CMD(ST7735S_GAMCTRN1), 0x0f, 0x1b, 0x0f, 0x17, 0x33, 0x2c, 0x29, 0x2e,
                         0x30, 0x30, 0x39, 0x3f, 0x00, 0x07, 0x03, 0x10,
  CMD(0xf0), 0x01,
  CMD(0xf6), 0x00,
  CMD(ST7735S_COLMOD), 0x05,
  CMD(ST7735S_MADCTL), 0xa0,
};


void lcd_init(void){

	HAL_GPIO_WritePin(lcd_rst_GPIO_Port, lcd_rst_Pin, 1); //nie ma resetu
	HAL_Delay(200);
	HAL_GPIO_WritePin(lcd_rst_GPIO_Port, lcd_rst_Pin, 0); //stan resetu
	HAL_Delay(200);
	HAL_GPIO_WritePin(lcd_rst_GPIO_Port, lcd_rst_Pin, 1); //nie ma resetu

	for(int i=0; i < sizeof(init_table) / sizeof(uint16_t); i++ ){  // i < (rozmiar tabeli w bajtach) podzielony (na dwa bajty) = ilosc elementow
		lcd_send(init_table[i]);
	}
	HAL_Delay(200);

	  lcd_command(ST7735S_SLPOUT);  	//sleep out
	  HAL_Delay(120);
	  lcd_command(ST7735S_DISPON); 		// display on
}


// my mamy little endian w cortexie wiec ta funkcja konwertuje jakby na big endian bo najpier przesuwa na starsze
//jesli uzywamy DMA to  nie uzywamy tego jakby tylko odrazu bierzemy dane a tam mimo ze zapisalismy w kolejnosci jak chcemy procesor najpierw i tak interpretuje od młodszej strony
static void lcd_data16(uint16_t value)
{
	lcd_data(value >> 8);		//podajemy 16 bity a wpisuje przesuniety o 8. Przyklad analogiczny ale dla 8: jesli mamy 22221111 to bedzie 00002222 i to czyta jako 2222
	lcd_data(value); 			//a tutaj dajemy 22221111 , ale wpisze sie 1111 bo czyta tylko tyle jaki ma format a lcd data jest 8 bit
}

//@pos_x- pozycja osi x, pos_y pozycja osi y, width- szerokosc obrazka, height- wysokosc obrazka
//podajemy x,y czyli skad zaczynamy a potem width,height czyli wielkosc
//sluzy tylko do wyznaczanie obszaru w ktorym potem zapisujemy cokolwiek chcemy
static void lcd_set_window(int pos_x, int pos_y, int width, int height){

	lcd_command(ST7735S_CASET);
	lcd_data16(1 + pos_x);
	lcd_data16(1 + pos_x + width -1); // 1+ x bo tak jest w sterowniku a potem -1 bo zaczynamy od zera wiec maks liczba to X-1

	lcd_command(ST7735S_RASET);
	lcd_data16(1 + pos_y);
	lcd_data16(1 + pos_y + height -1);
}
/*
//nie jest staic bo jest dla uzytkownika
void lcd_fill_box(int x, int y, int width, int height, uint16_t color)
{
	lcd_set_window(x, y, width, height);
	lcd_command(ST7735S_RAMWR);
	for (int i = 0; i < width * height; i++)
		lcd_data16(color);
}

//nowa zmieniona niżej

void lcd_put_pixel(int x, int y, uint16_t color)
{
  lcd_fill_box(x, y, 1, 1, color);
}
*/

/*
void lcd_draw_image(int x, int y, int width, int height, const uint8_t* data)	//uint8_t bo spi transmit to 8 bitów
{
	lcd_set_window(x, y, width, height);

	lcd_command(ST7735S_RAMWR);
	używajac tej metody co jeden bit machamy dc i cs co jest nie potrzebne, bo można to zrobic raz i przekazac adres do tablicy i za jednym rozkazem wyslac dane
	//for (int i = 0; i < width * height * 2; i++)
	//	lcd_data(data[i]);

	HAL_GPIO_WritePin(lcd_dc_GPIO_Port, lcd_dc_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(lcd_cs_GPIO_Port, lcd_cs_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, (uint8_t*)data, width * height * 2, HAL_MAX_DELAY);  //  *2 bo transmitujemy 8bitow a kazdy kolor wazy 16, HAL_SPI_Transmit uzyc potem HAL_SPI_Transmit_DMA(...) bo nie mozemy miec dluego delay w rtos
	HAL_GPIO_WritePin(lcd_cs_GPIO_Port, lcd_cs_Pin, GPIO_PIN_SET);
}
*/

static uint16_t frame_buffer[LCD_WIDTH * LCD_HEIGHT];

void lcd_put_pixel(void *surface,int x, int y, uint16_t color)
{
	if (x < 0 || x >= LCD_WIDTH || y < 0 || y >= LCD_HEIGHT) {
	        return;
	    }
	frame_buffer[x + y * LCD_WIDTH] = color;	// ekran jest 2D więc ma wspolrzedne x i y a my przerabiamy go na 1D więc mamy (x_wierszy + y_kolumny * szerokosc) = miejsce tego piksela z perspektywy tablicy 1 wymiarowej
}


void lcd_copy(void)
{
	lcd_set_window(0,0, LCD_WIDTH, LCD_HEIGHT);	// CAŁY EKRAN
	lcd_command(ST7735S_RAMWR);
	HAL_GPIO_WritePin(lcd_dc_GPIO_Port, lcd_dc_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(lcd_cs_GPIO_Port, lcd_cs_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit_DMA(&hspi1, (uint8_t*)frame_buffer, sizeof(frame_buffer));
	//HAL_GPIO_WritePin(lcd_cs_GPIO_Port, lcd_cs_Pin, GPIO_PIN_SET);

}



void lcd_transfer_done(void)
{
	HAL_GPIO_WritePin(lcd_cs_GPIO_Port, lcd_cs_Pin, GPIO_PIN_SET);
}


bool lcd_is_busy(void)
{
	if (HAL_SPI_GetState(&hspi1) == HAL_SPI_STATE_BUSY)
		return true;
	else
		return false;
}






















