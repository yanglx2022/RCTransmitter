/**********************
* LED
**********************/
#ifndef __LED_H
#define __LED_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "gd32f1x0.h"

#define LED_RED		0x01
#define LED_BLUE	0x02

#define LED_0			0x01
#define LED_1			0x02
#define LED_2			0x04
#define LED_3			0x08
#define LED_ALL		0x0F

void Led_Init(void);
void Led_Top(uint8_t color, uint8_t led);
void Led_Bottom(uint8_t color, uint8_t led);


#ifdef __cplusplus
}
#endif
#endif /*__LED_H */


