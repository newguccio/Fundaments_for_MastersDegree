#pragma once // plik nagłówkowy nie będzie włączany więcej niż raz do innych plików

#include <stdint.h>
#include <stdbool.h>

#define LCD_WIDTH	160
#define LCD_HEIGHT	129


#define little_endian_DMA 		1
// #define little_endian_on		1

#ifdef little_endian_on
#define BLACK     0x0000
#define RED       0xf800
#define GREEN     0x07e0
#define BLUE      0x001f
#define YELLOW    0xffe0
#define MAGENTA   0xf81f
#define CYAN      0x07ff
#define WHITE     0xffff
#endif

//dla DMA po prostu zamieniamy kolejnosc bajtow
#if little_endian_DMA
#define BLACK			0x0000
#define RED				0x00f8
#define GREEN			0xe007
#define BLUE			0x1f00
#define YELLOW			0xe0ff
#define MAGENTA			0x1ff8
#define CYAN			0xff07
#define WHITE			0xffff
#endif

void lcd_init(void);

void lcd_fill_box(int x, int y, int width, int height, uint16_t color);

void lcd_put_pixel(void *surface,int x, int y, uint16_t color);

void lcd_draw_image(int x, int y, int width, int height, const uint8_t* data);

void lcd_copy(void);

void lcd_transfer_done(void);
bool lcd_is_busy(void);
