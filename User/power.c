/**********************
* ���ػ�
**********************/
#include "power.h"
#include "delay.h"
#include "adc.h"
#include "led.h"
#include "beep.h"

// ����
void Power_On(void)
{
	delay_init();
	delay_ms(1000);	//������������1s����
	// PA15���ƿ��ػ�
	rcu_periph_clock_enable(RCU_GPIOA);
	gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_15);
	gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_15);
	gpio_bit_set(GPIOA, GPIO_PIN_15);	// ���ֿ���
	// PA12���ػ���ť
	gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_12);
	// ��������һ��
	Beep_Init();
	Beep(100);
	delay_ms(110);
}

// �ػ�
void Power_Off(void)
{
	// �ر�ADC
	ADC_Deinit();
	// �ر�����LED
	Led_Top(0, 0);
	Led_Bottom(0, 0);
	// ����
	Beep(100);
	delay_ms(110);
	// �ϵ�
	gpio_bit_reset(GPIOA, GPIO_PIN_15);
	// �ȴ���ţ̌���Դ�Ͽ�
	while(1);
}

// ���ػ���ť״̬
uint8_t Power_Key(void)
{
	return gpio_input_bit_get(GPIOA, GPIO_PIN_12);
}



