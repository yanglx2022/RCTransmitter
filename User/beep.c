/**********************
* BEEP
**********************/
#include "beep.h"

// 蜂鸣器2.7kHz(T=370us)
#define TIM_PERIOD_T1			370		// 370us
#define TIM_PRESCALER_T1	16

static volatile int32_t cycle_cnt = 0;	// 周期计数

// 初始化
void Beep_Init(void)
{
	// PB10
	rcu_periph_clock_enable(RCU_GPIOB);
	gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_10);
	gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_10);
	gpio_af_set(GPIOB, GPIO_AF_2, GPIO_PIN_10);
	// TIM1
	rcu_periph_clock_enable(RCU_TIMER1);
	timer_deinit(TIMER1);
	timer_parameter_struct timer_struct;
	timer_struct.alignedmode 			 = TIMER_COUNTER_EDGE;
	timer_struct.clockdivision 		 = TIMER_CKDIV_DIV1;
	timer_struct.counterdirection  = TIMER_COUNTER_UP;
	timer_struct.period 					 = TIM_PERIOD_T1 - 1;
	timer_struct.prescaler 				 = TIM_PRESCALER_T1 - 1;
	timer_struct.repetitioncounter = 0;
	timer_init(TIMER1, &timer_struct);
	timer_auto_reload_shadow_enable(TIMER1);
	// TIM1_CH2 PWM
	timer_oc_parameter_struct timer_oc_struct;
	timer_channel_output_struct_para_init(&timer_oc_struct);
	timer_oc_struct.outputstate  = TIMER_CCX_ENABLE;
	timer_oc_struct.outputnstate = TIMER_CCXN_DISABLE;
	timer_oc_struct.ocpolarity   = TIMER_OC_POLARITY_HIGH;
	timer_channel_output_config(TIMER1, TIMER_CH_2, &timer_oc_struct);
	timer_channel_output_mode_config(TIMER1, TIMER_CH_2, TIMER_OC_MODE_PWM1); // MODE_PWM1以保证蜂鸣停后为断电状态以避免发热
	timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_2, TIM_PERIOD_T1 / 2); // 50%占空比
	// TIM1中断
	timer_interrupt_flag_clear(TIMER1, TIMER_INT_FLAG_UP);
	timer_interrupt_enable(TIMER1, TIMER_INT_UP);
	nvic_irq_enable(TIMER1_IRQn, 0, 0);
}

// 蜂鸣
void Beep(int32_t time_ms)
{
	cycle_cnt = time_ms * 1000 / TIM_PERIOD_T1;
	if (cycle_cnt > 0)
	{
		timer_enable(TIMER1);
	}
}

// 定时中断
void TIMER1_IRQHandler(void)
{
	if (timer_interrupt_flag_get(TIMER1, TIMER_INT_FLAG_UP) == SET)
	{
		cycle_cnt--;
		if (cycle_cnt <= 0)
		{
			timer_disable(TIMER1);
		}
		timer_interrupt_flag_clear(TIMER1, TIMER_INT_FLAG_UP);
	}
}

