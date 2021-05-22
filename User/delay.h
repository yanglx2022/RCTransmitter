/**********************
*系统tick延时
**********************/
#ifndef __DELAY_H
#define __DELAY_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "gd32f1x0.h"

void Delay_Init(void);
void delay_us(uint16_t num);
void delay_ms(uint32_t num);


#ifdef __cplusplus
}
#endif
#endif /*__DELAY_H */
