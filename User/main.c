// 2.4G遥控器
#include "delay.h"
#include "key.h"
#include "led.h"
#include "beep.h"
#include "bk2425.h"
#include "print.h"

int main(void)
{
	// 长按1s开机
	Delay_Init();
	delay_ms(1000);
	// PA15控制开关机
	rcu_periph_clock_enable(RCU_GPIOA);
	gpio_mode_set(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_15);
	gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_15);
	gpio_bit_set(GPIOA, GPIO_PIN_15);	// 保持开机
	// 开机蜂鸣一声
	Beep_Init();
	Beep(100);
	delay_ms(110);
	// 设置中断优先级分组
	nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
	// 串口初始化
	Print_Init();
	// 无线模块初始化(失败尝试3次)
	for(int i = 0; i < 3; i++)
	{
		if (BK2425_Init())	// 无线模块初始化
		{
			break;
		}
		delay_ms(10);
	}
	// LED初始化
	Led_Init();
	// 摇杆等主业务初始化
	Key_Init();
	while(1);
}
