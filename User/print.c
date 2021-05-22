/**********************
* 串口打印  USART0  PA9
**********************/
#include "print.h"

// 重定义fputc函数
int fputc(int ch, FILE *f)
{
	while(usart_flag_get(USART0, USART_FLAG_TC) == RESET);
	usart_data_transmit(USART0, ch);
	return ch;
}

// 串口初始化
void Print_Init(void)
{
	// IO初始化
	rcu_periph_clock_enable(RCU_GPIOA);
	gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_9);
	gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
	gpio_af_set(GPIOA, GPIO_AF_1, GPIO_PIN_9);
	// USART0初始化
	rcu_periph_clock_enable(RCU_USART0);
	usart_deinit(USART0);
	usart_baudrate_set(USART0, 115200);
	usart_parity_config(USART0, USART_PM_NONE);
	usart_word_length_set(USART0, USART_WL_8BIT);
	usart_stop_bit_set(USART0, USART_STB_1BIT);
	usart_enable(USART0);
	usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
}


