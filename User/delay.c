/**********************
*系统tick延时
**********************/
#include "delay.h"

// 初始化(利用SysTick计数值产生随机种子,因此上电开始一直计数)
void delay_init(void)
{
	SysTick->CTRL  = 0UL;				// 使用外部时钟(HCLK/8=2MHz) 不开中断
	SysTick->LOAD  = 0xFFFFFF;	// 最大计数
	SysTick->VAL 	 = 0UL;				// 清空计数器
	SysTick->CTRL |= 1UL;				// 开始计数
}

// us延时(最大延时2^16us<SysTick溢出时间2^24/2=2^23us,因此只处理一次溢出即可)
void delay_us(uint16_t num)
{
	uint32_t start = SysTick->VAL + 0x1000000;
	uint32_t now = start;
	num *= 2;
	while(((start - now) % 0x1000000) < num)
	{
		now = SysTick->VAL;
	}
}

// ms延时
void delay_ms(uint32_t num)
{
	uint32_t repeat = num / 60;
	uint32_t remain = num % 60;
	while(repeat--)
	{
		delay_us(60000);
	}
	if (remain)
	{
		delay_us(remain * 1000);
	}
}

// tick
uint32_t delay_tick(void)
{
	return SysTick->VAL;
}


