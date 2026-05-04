/*
 * hagl_hal_color.h
 *
 *  Created on: 3 maj 2026
 *      Author: hasom
 */

#ifndef HAGL_HAL_COLOR_H_
#define HAGL_HAL_COLOR_H_

#include "lcd.h"

#include <stdint.h>

#define DISPLAY_WIDTH 	(LCD_WIDTH)
#define DISPLAY_HEIGHT 	(LCD_HEIGHT)
#define DISPLAY_DEPTH 	16

typedef uint16_t hagl_color_t;



#define hagl_hal_init(x)		 	(0)
#define hagl_hal_put_pixel 			lcd_put_pixel


#endif /* HAGL_HAL_COLOR_H_ */

