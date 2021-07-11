/**********************
* 摇杆与按键  ADC
**********************/
#ifndef __ADC_H
#define __ADC_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "gd32f1x0.h"

// 扫描周期
#define TIM_PERIOD_T2					 22000	// 22ms
#define TIM_PRESCALER_T2			 16			// 1us(16MHz主频)

// 新数据标志
extern uint8_t adc_achieve;

void ADC_Init(void);
void ADC_Deinit(void);
void ADC_Parse(uint8_t* parsed_data);
uint32_t ADC_Voltage(void);


#ifdef __cplusplus
}
#endif
#endif /*__ADC_H */


