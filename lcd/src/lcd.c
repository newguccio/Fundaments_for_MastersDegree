#include <stdio.h>

#include "lcd.h"
#include "spi.h"

//makra z datasheetu ekranu ktore odpowiadaja za komunikacje z wyswietlaczem
#define ST7735S_SLPOUT			0x11
#define ST7735S_DISPOFF			0x28
#define ST7735S_DISPON			0x29
#define ST7735S_CASET			0x2a
#define ST7735S_RASET			0x2b
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


static void lcd_command(uint8_t command){

	HAL_GPIO_WritePin(lcd_dc_GPIO_Port, lcd_dc_Pin, 0); // zamiast 0 mozna dac GPIO_PIN_RESET ale dla mnie tak czytelniej jest jak jest tu
	HAL_GPIO_WritePin(lcd_cs_GPIO_Port, lcd_cs_Pin, 0);	//
	HAL_SPI_Transmit(hspi1, &command, 1, 10);
	HAL_GPIO_WritePin(lcd_cs_GPIO_Port, lcd_cs_Pin, 1);

}

static void lcd_data(uint8_t data){

	HAL_GPIO_WritePin(lcd_dc_GPIO_Port, lcd_dc_Pin, 1); // zamiast 0 mozna dac GPIO_PIN_RESET ale dla mnie tak czytelniej jest jak jest tu
	HAL_GPIO_WritePin(lcd_cs_GPIO_Port, lcd_cs_Pin, 0);	//
	HAL_SPI_Transmit(hspi1, &data, 1, 10);
	HAL_GPIO_WritePin(lcd_cs_GPIO_Port, lcd_cs_Pin, 1);

}

static void lcd_send(uint16_t value){ 	//bierze wartosc i sprawdza czy jest 1 na 9 pozycji i w zalezsnoci od tego przechodzi do funkcji od komend albo danych

	if(value & 0x100){					//sprawdza 9 pozycje
		lcd_command(value);				//jesli jest 1 to ==command wiec wysylam jako komende
	}else{
		lcd_data(value);
	}

}

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

	  lcd_cmd(ST7735S_SLPOUT);  	//sleep out
	  HAL_Delay(120);
	  lcd_cmd(ST7735S_DISPON); 		// display on
}

















