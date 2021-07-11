/**********************
*系统tick延时
**********************/
#ifndef __DELAY_H
#define __DELAY_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "gd32f1x0.h"

// 判断超时(只能判断最多一次计时溢出的情况)
#define TICK_TIMEOUT(start, timeout)	((((start) + 0x1000000 - SysTick->VAL) % 0x1000000) > (timeout))

void delay_init(void);
void delay_us(uint16_t num);
void delay_ms(uint32_t num);
uint32_t delay_tick(void);


#ifdef __cplusplus
}
#endif
#endif /*__DELAY_H */
