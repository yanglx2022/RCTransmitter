/**********************
* 开关机
**********************/
#include "power.h"
#include "delay.h"
#include "adc.h"
#include "led.h"
#include "beep.h"

// 开机
void Power_On(void)
{
	delay_init();
	delay_ms(1000);	//长按开机按键1s开机
	// PA15控制开关机
	rcu_periph_clock_enable(RCU_GPIOA);
	gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_15);
	gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_15);
	gpio_bit_set(GPIOA, GPIO_PIN_15);	// 保持开机
	// PA12开关机按钮
	gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_12);
	// 开机蜂鸣一声
	Beep_Init();
	Beep(100);
	delay_ms(110);
}

// 关机
void Power_Off(void)
{
	// 关闭ADC
	ADC_Deinit();
	// 关闭所有LED
	Led_Top(0, 0);
	Led_Bottom(0, 0);
	// 蜂鸣
	Beep(100);
	delay_ms(110);
	// 断电
	gpio_bit_reset(GPIOA, GPIO_PIN_15);
	// 等待按钮抬起电源断开
	while(1);
}

// 开关机按钮状态
uint8_t Power_Key(void)
{
	return gpio_input_bit_get(GPIOA, GPIO_PIN_12);
}



