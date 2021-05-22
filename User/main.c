// 2.4Gң����
#include "delay.h"
#include "key.h"
#include "led.h"
#include "beep.h"
#include "bk2425.h"
#include "print.h"

int main(void)
{
	// ����1s����
	Delay_Init();
	delay_ms(1000);
	// PA15���ƿ��ػ�
	rcu_periph_clock_enable(RCU_GPIOA);
	gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_15);
	gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_15);
	gpio_bit_set(GPIOA, GPIO_PIN_15);	// ���ֿ���
	// ��������һ��
	Beep_Init();
	Beep(100);
	delay_ms(110);
	// �����ж����ȼ�����
	nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
	// ���ڳ�ʼ��
	Print_Init();
	// ����ģ���ʼ��(ʧ�ܳ���3��)
	for(int i = 0; i < 3; i++)
	{
		if (BK2425_Init())	// ����ģ���ʼ��
		{
			break;
		}
		delay_ms(10);
	}
	// LED��ʼ��
	Led_Init();
	// ҡ�˵���ҵ���ʼ��
	Key_Init();
	while(1);
}
