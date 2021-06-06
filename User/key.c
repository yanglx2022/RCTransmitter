/**********************
* ҡ���밴��/����  ADC
**********************/
#include "key.h"
#include "led.h"
#include "beep.h"
#include "delay.h"
#include "bk2425.h"
#include "print.h"

// ɨ������
#define TIM_PERIOD_T2			22000	// 22ms
#define TIM_PRESCALER_T2	16
static const uint32_t CNT_1S = 16000000 / (TIM_PERIOD_T2 * TIM_PRESCALER_T2);

// ����ͨ��֡����1Hz
static uint8_t reduce_fps = 0;

// ����80% 60% 40% 20% 5%��Ӧ�ĵ�ѹ TODO��ȷ��
static const uint32_t BAT_MV[5] = {5500, 5000, 4500, 4000, 3625};	// ��������6Vû��3.5V�ҵ������ѹ����

// ADCͨ������
#define ADC_CH_NUM				7
static uint16_t adc_data[ADC_CH_NUM] = {0};

// ҡ��AD��Χ(ʵ��)
static const uint16_t RANGE_LEFT_X[4]  = {460, 1700, 1900, 3500};
static const uint16_t RANGE_LEFT_Y[4]  = {550, 2000, 2100, 3650};
static const uint16_t RANGE_RIGHT_X[4] = {600, 1920, 2120, 3620};
static const uint16_t RANGE_RIGHT_Y[4] = {420, 1900, 2100, 3500};

static void gpio_init(void);
static void tim2_init(void);
static void adc_dma_init(void);
static void adc_adc_init(void);

// ��ʼ��
void Key_Init(void)
{
	gpio_init();
	adc_dma_init();
	adc_adc_init();
	tim2_init();
}

// GPIO
void gpio_init(void)
{
	// PA12 ���ػ���ť
	rcu_periph_clock_enable(RCU_GPIOA);
	gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_12);
	// PB2 ��ҡ�������Զ�����״̬
	rcu_periph_clock_enable(RCU_GPIOB);
	gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_2);
}

// TIM2
void tim2_init(void)
{
	rcu_periph_clock_enable(RCU_TIMER2);
	timer_deinit(TIMER2);
	timer_parameter_struct timer_struct;
	timer_struct.alignedmode 			 = TIMER_COUNTER_EDGE;
	timer_struct.clockdivision 		 = TIMER_CKDIV_DIV1;
	timer_struct.counterdirection  = TIMER_COUNTER_UP;
	timer_struct.period 					 = TIM_PERIOD_T2 - 1;
	timer_struct.prescaler 				 = TIM_PRESCALER_T2 - 1;
	timer_struct.repetitioncounter = 0;
	timer_init(TIMER2, &timer_struct);
	timer_auto_reload_shadow_enable(TIMER2);
	timer_master_output_trigger_source_select(TIMER2, TIMER_TRI_OUT_SRC_UPDATE);
	timer_enable(TIMER2);
}

// ADC DMA
void adc_dma_init(void)
{
	rcu_periph_clock_enable(RCU_DMA);
	dma_deinit(DMA_CH0);
	dma_parameter_struct dma_struct;
	dma_struct.periph_addr 	= (uint32_t)&ADC_RDATA;
	dma_struct.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
	dma_struct.periph_inc 	= DMA_PERIPH_INCREASE_DISABLE;
	dma_struct.memory_addr 	= (uint32_t)adc_data;
	dma_struct.memory_width = DMA_MEMORY_WIDTH_16BIT;
	dma_struct.memory_inc 	= DMA_MEMORY_INCREASE_ENABLE;
	dma_struct.direction 		= DMA_PERIPHERAL_TO_MEMORY;
	dma_struct.number 			= ADC_CH_NUM;
	dma_struct.priority 		= DMA_PRIORITY_HIGH;
	dma_init(DMA_CH0, &dma_struct);
	dma_circulation_enable(DMA_CH0);
	dma_interrupt_flag_clear(DMA_CH0, DMA_INT_FLAG_FTF);
	dma_interrupt_enable(DMA_CH0, DMA_INT_FTF);
	nvic_irq_enable(DMA_Channel0_IRQn, 1, 0);
	dma_channel_enable(DMA_CH0);
}

// ADC
void adc_adc_init(void)
{
	// IO��ʼ��
	rcu_periph_clock_enable(RCU_GPIOA);
	gpio_mode_set(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, 
		GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
	rcu_periph_clock_enable(RCU_GPIOB);
	gpio_mode_set(GPIOB, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_1);
	// ADC��ʼ��
	rcu_periph_clock_enable(RCU_ADC);
	rcu_adc_clock_config(RCU_ADCCK_APB2_DIV8);
	adc_deinit();
	adc_dma_mode_enable();
	adc_special_function_config(ADC_CONTINUOUS_MODE, DISABLE);
	adc_special_function_config(ADC_SCAN_MODE, ENABLE);
	adc_data_alignment_config(ADC_DATAALIGN_RIGHT);
	adc_channel_length_config(ADC_REGULAR_CHANNEL, ADC_CH_NUM);
	adc_regular_channel_config(0, ADC_CHANNEL_0, ADC_SAMPLETIME_239POINT5);
	adc_regular_channel_config(1, ADC_CHANNEL_1, ADC_SAMPLETIME_239POINT5);
	adc_regular_channel_config(2, ADC_CHANNEL_4, ADC_SAMPLETIME_239POINT5);
	adc_regular_channel_config(3, ADC_CHANNEL_5, ADC_SAMPLETIME_239POINT5);
	adc_regular_channel_config(4, ADC_CHANNEL_6, ADC_SAMPLETIME_239POINT5);
	adc_regular_channel_config(5, ADC_CHANNEL_7, ADC_SAMPLETIME_239POINT5);
	adc_regular_channel_config(6, ADC_CHANNEL_9, ADC_SAMPLETIME_239POINT5);
	adc_external_trigger_source_config(ADC_REGULAR_CHANNEL, ADC_EXTTRIG_REGULAR_T2_TRGO);
	adc_external_trigger_config(ADC_REGULAR_CHANNEL, ENABLE);
	adc_enable();
	delay_us(100);	// ��ʱ����14��ADCCLK�Եȴ�ADC�ȶ�
	adc_calibration_enable();
}

// ��ADCֵת����-200~200(��������)
static int16_t convert(uint16_t value, const uint16_t* range)
{
	if (value <= range[0])
	{
		return -200;
	}
	else if (value < range[1])
	{
		return 200 * ((int16_t)value - range[1]) / (range[1] - range[0]);
	}
	else if (value <= range[2])
	{
		return 0;
	}
	else if (value < range[3])
	{
		return 200 * ((int16_t)value - range[2]) / (range[3] - range[2]);
	}
	else
	{
		return 200;
	}
}

// ��ADCֵת����0~400
static int16_t convert1(uint16_t value, const uint16_t* range)
{
	uint16_t range0 = range[0];
	if (value <= range0)
	{
		return 0;
	}
	else if (value < range[3])
	{
		return 400 * (value - range0) / (range[3] - range0);
	}
	else
	{
		return 400;
	}
}

// ��ADCֵת��Ϊ����״̬
static uint8_t key_convert(uint16_t value)
{
	if (value < 4096 * 1 / 8)
	{
		return 0x01;
	}
	else if (value < 4096 * 3 / 8)
	{
		return 0x02;
	}
	else if (value < 4096 * 5 / 8)
	{
		return 0x04;
	}
	else if (value < 4096 * 7 / 8)
	{
		return 0x08;
	}
	return 0x00;
}

// �ػ�
static void power_off(void)
{
	// �ر�ҡ�˰���
	dma_channel_disable(DMA_CH0);
	adc_disable();
	timer_disable(TIMER2);
	// �ر�����LED
	Led_Top(0, 0);
	Led_Bottom(0, 0);
	// ����
	Beep(100);
	delay_ms(100);
	// �ϵ�
	gpio_bit_reset(GPIOA, GPIO_PIN_15);
	// �ȴ���ţ̌���Դ�Ͽ�
	while(1);
}

// �������LEDָʾ
static void transmit_display(uint8_t* data, uint8_t transmit)
{
	// ���߳�ʼ��ʧ��
	if (data == 0)
	{
		static uint32_t bk_cnt = 0;
		bk_cnt++;
		// ����LEDһ����˸10s��ػ�
		if (bk_cnt % (CNT_1S / 2) == 0)
		{
			Led_Top(LED_ALL, LED_ALL);
		}
		else if (bk_cnt % (CNT_1S / 4) == 0)
		{
			Led_Top(0, 0);
		}
		if (bk_cnt >= CNT_1S * 10)
		{
			power_off();
		}
	}
	else
	{
		static uint8_t idle = 0, fail = 0;	// �ػ���־
		static uint32_t power_cnt = 0;
		if (fail == 0)
		{
			// ҡ��ȫĬ��״̬
			static uint32_t idle_cnt = 0;
			if (data[0] == 0 && 							// �ް�������
				*(uint32_t*)(data + 2) == 0 &&	// ��ҡ��0λ
				*(uint32_t*)(data + 6) == 0)		// ��ҡ��0λ
			{
				idle_cnt++;
				if (idle_cnt >= CNT_1S * 1770)	// 29min30s
				{
					idle = 1;
				}
			}
			else
			{
				idle_cnt = 0;
				idle = 0;
				power_cnt = 0;
			}
		}
		if (idle == 0)
		{
			// ����ʧ��
			static uint32_t fail_cnt = 0;
			if (transmit == 0)
			{
				fail_cnt++;
				if (fail_cnt >= CNT_1S * 5)
				{
					reduce_fps = 1;	// ��������ʧ��5s�򽵵�ͨ��֡����1Hz
				}
				if (fail_cnt >= CNT_1S * 270)	// 4min30s
				{
					fail = 1;
				}
			}
			else
			{
				fail_cnt = 0;
				fail = 0;
				power_cnt = 0;
			}
		}
		// LEDָʾ
		if (idle || fail)
		{
			// �Ϻ�LED��˸30sȻ��ػ�
			power_cnt++;
			if (power_cnt % (CNT_1S / 2) == 0)
			{
				Led_Top(LED_RED, LED_ALL);
			}
			else if (power_cnt % (CNT_1S / 4) == 0)
			{
				Led_Top(0, 0);
			}
			if (power_cnt >= CNT_1S * 30)
			{
				power_off();
			}
		}
		else
		{
			// ����״ָ̬ʾ
			if (transmit)
			{
				Led_Top(LED_BLUE, LED_ALL);	// ���ͳɹ�����ɫLED��
				reduce_fps = 0;	// �ָ�����֡��
			}
			else
			{
				Led_Top(LED_RED, LED_ALL);	// ����ʧ���Ϻ�ɫLED��
			}
		}
	}
}

// ���ݵ�ѹ��ʾ����
static void battery_display(uint32_t voltage)
{
	if (voltage >= BAT_MV[0])				// 80% 4����LED
	{
		Led_Bottom(LED_BLUE, LED_ALL);
	}
	else if (voltage >= BAT_MV[1])	// 60% 3����LED
	{
		Led_Bottom(LED_BLUE, LED_0 | LED_1 | LED_2);
	}
	else if (voltage >= BAT_MV[2])	// 40% 2����LED
	{
		Led_Bottom(LED_BLUE, LED_0 | LED_1);
	}
	else if (voltage >= BAT_MV[3])	// 20% 1����LED
	{
		Led_Bottom(LED_BLUE, LED_0);
	}
	else if (voltage >= BAT_MV[4])	// 5% 1����LED
	{
		Led_Bottom(LED_RED, LED_1);
	}
	else														// <5% �²����к�LED��˸
	{
		static uint32_t cnt = 0;
		cnt++;
		if (cnt == CNT_1S)
		{
			Led_Bottom(LED_RED, LED_ALL);
		}
		else if (cnt == CNT_1S * 2)
		{
			Led_Bottom(0, 0);
			cnt = 0;
		}
	}
}

// DMA�ж�
void DMA_Channel0_IRQHandler(void)
{
	if (dma_interrupt_flag_get(DMA_CH0, DMA_INT_FLAG_FTF) == SET)
	{
		// ����ҡ�˰���״̬
		if (BK2425_IsReady())
		{
			//printf("%d\t%d\t%d\t%d\t", adc_data[2], adc_data[3], adc_data[6], adc_data[0]);
			uint8_t data[PAYLOAD_WIDTH];
			data[0] = gpio_input_bit_get(GPIOB, GPIO_PIN_2);											// ��ҡ��Y�Զ����б�־
			data[1] = key_convert(adc_data[4]) | (key_convert(adc_data[1]) << 4);	// ����
			data[1] = (data[1] & 0x3F) | ((data[1] >> 1) & 0x40) | ((data[1] << 1) & 0x80);
			*(int16_t*)(data + 2) = -convert(adc_data[2], RANGE_LEFT_X);						// ��ҡ��X
			*(int16_t*)(data + 4) = data[0] ? convert(adc_data[3], RANGE_LEFT_Y) : 
																				convert1(adc_data[3], RANGE_LEFT_Y);// ��ҡ��Y
			*(int16_t*)(data + 6) = convert(adc_data[6], RANGE_RIGHT_X);					// ��ҡ��X
			*(int16_t*)(data + 8) = -convert(adc_data[0], RANGE_RIGHT_Y);					// ��ҡ��Y
			uint8_t ret = 0;
			if (reduce_fps)
			{
				static uint32_t transmit_cnt = 0;
				transmit_cnt++;
				if (transmit_cnt >= CNT_1S)
				{
					transmit_cnt = 0;
					ret = BK2425_Transmit(data);	// ����
				}
			}
			else
			{
				ret = BK2425_Transmit(data);		// ����
			}
			transmit_display(data, ret);
		}
		else
		{
			transmit_display(0, 0);
		}
		// ��ص���
		uint32_t voltage_mv = (uint32_t)adc_data[5] * 3300 * 3 / 4095;
		battery_display(voltage_mv);
		// ��ⳤ���ػ�
		static uint32_t power_off_cnt = 0;
		static uint8_t power_off_enable = 0;
		if (gpio_input_bit_get(GPIOA, GPIO_PIN_12))	// ���¿��ػ�����
		{
			if (power_off_enable)	// ���⿪������ʱ������ִ����ػ�
			{
				power_off_cnt++;
				if (power_off_cnt >= CNT_1S)
				{
					power_off();
				}
			}
		}
		else
		{
			power_off_cnt = 0;
			power_off_enable = 1;
		}
		dma_interrupt_flag_clear(DMA_CH0, DMA_INT_FLAG_FTF);
	}
}



