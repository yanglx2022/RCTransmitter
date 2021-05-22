/**********************
* LED
**********************/
#include "led.h"

// 初始化
void Led_Init(void)
{
	// PA11
	rcu_periph_clock_enable(RCU_GPIOA);
	gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_11);
	gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_11);
	// PB8 PB9 PB11~15
	rcu_periph_clock_enable(RCU_GPIOB);
	gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, 
		GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
	gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, 
		GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
	// PC13~15
	rcu_periph_clock_enable(RCU_GPIOC);
	gpio_mode_set(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
	gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);
	// 关闭所有LED
	Led_Top(0, 0);
	Led_Bottom(0, 0);
}

// 上部LED
void Led_Top(uint8_t color, uint8_t led)
{
	// 红
	if (color & LED_RED)
	{
		gpio_bit_reset(GPIOB, GPIO_PIN_9);
	}
	else
	{
		gpio_bit_set(GPIOB, GPIO_PIN_9);
	}
	// 蓝
	if (color & LED_BLUE)
	{
		gpio_bit_reset(GPIOB, GPIO_PIN_8);
	}
	else
	{
		gpio_bit_set(GPIOB, GPIO_PIN_8);
	}
	// 左
	if (led & LED_0)
	{
		gpio_bit_reset(GPIOC, GPIO_PIN_15);
	}
	else
	{
		gpio_bit_set(GPIOC, GPIO_PIN_15);
	}
	// 中
	if (led & LED_1)
	{
		gpio_bit_reset(GPIOC, GPIO_PIN_14);
	}
	else
	{
		gpio_bit_set(GPIOC, GPIO_PIN_14);
	}
	// 右
	if (led & LED_2)
	{
		gpio_bit_reset(GPIOC, GPIO_PIN_13);
	}
	else
	{
		gpio_bit_set(GPIOC, GPIO_PIN_13);
	}
}

// 下部LED
void Led_Bottom(uint8_t color, uint8_t led)
{
	// 红
	if (color & LED_RED)
	{
		gpio_bit_reset(GPIOB, GPIO_PIN_12);
	}
	else
	{
		gpio_bit_set(GPIOB, GPIO_PIN_12);
	}
	// 蓝
	if (color & LED_BLUE)
	{
		gpio_bit_reset(GPIOB, GPIO_PIN_11);
	}
	else
	{
		gpio_bit_set(GPIOB, GPIO_PIN_11);
	}
	// 上
	if (led & LED_0)
	{
		gpio_bit_reset(GPIOA, GPIO_PIN_11);
	}
	else
	{
		gpio_bit_set(GPIOA, GPIO_PIN_11);
	}
	// 右
	if (led & LED_1)
	{
		gpio_bit_reset(GPIOB, GPIO_PIN_15);
	}
	else
	{
		gpio_bit_set(GPIOB, GPIO_PIN_15);
	}
	// 下
	if (led & LED_2)
	{
		gpio_bit_reset(GPIOB, GPIO_PIN_13);
	}
	else
	{
		gpio_bit_set(GPIOB, GPIO_PIN_13);
	}
	// 左
	if (led & LED_3)
	{
		gpio_bit_reset(GPIOB, GPIO_PIN_14);
	}
	else
	{
		gpio_bit_set(GPIOB, GPIO_PIN_14);
	}
}



