/**********************
* ҡ���밴��/����  ADC
**********************/
#include "adc.h"
#include "delay.h"

// ADCͨ������
#define ADC_CH_NUM			7
uint16_t adc_data[ADC_CH_NUM] = {0};

// �����ݱ�־
uint8_t adc_achieve = 0;

// ҡ��AD��Χ(ʵ��)
static const uint16_t RANGE_LEFT_X[4]  = {460, 1700, 1900, 3500};
static const uint16_t RANGE_LEFT_Y[4]  = {550, 2000, 2100, 3650};
static const uint16_t RANGE_RIGHT_X[4] = {600, 1920, 2120, 3620};
static const uint16_t RANGE_RIGHT_Y[4] = {420, 1900, 2100, 3500};


static void gpio_init(void);
static void tim2_init(void);
static void adc_dma_init(void);
static void adc_adc_init(void);
static int16_t convert(uint16_t value, const uint16_t* range);
static int16_t convert1(uint16_t value, const uint16_t* range);
static uint8_t key_convert(uint16_t value);

// ��ʼ��
void ADC_Init(void)
{
	gpio_init();
	adc_dma_init();
	adc_adc_init();
	tim2_init();
}

// �ر�
void ADC_Deinit(void)
{
	dma_channel_disable(DMA_CH0);
	adc_disable();
	timer_disable(TIMER2);
}

// ADC���ݽ���
void ADC_Parse(uint8_t* parsed_data)
{
	parsed_data[0] = gpio_input_bit_get(GPIOB, GPIO_PIN_2);												// ��ҡ��Y�Զ����б�־
	parsed_data[1] = key_convert(adc_data[4]) | (key_convert(adc_data[1]) << 4);	// ����
	parsed_data[1] = (parsed_data[1] & 0x3F) | ((parsed_data[1] >> 1) & 0x40) | ((parsed_data[1] << 1) & 0x80);
	*(int16_t*)(parsed_data + 2) = -convert(adc_data[2], RANGE_LEFT_X);						// ��ҡ��X
	*(int16_t*)(parsed_data + 4) = parsed_data[0] ? convert(adc_data[3], RANGE_LEFT_Y) : 
																		convert1(adc_data[3], RANGE_LEFT_Y);				// ��ҡ��Y
	*(int16_t*)(parsed_data + 6) = convert(adc_data[6], RANGE_RIGHT_X);						// ��ҡ��X
	*(int16_t*)(parsed_data + 8) = -convert(adc_data[0], RANGE_RIGHT_Y);					// ��ҡ��Y
}

// ��ȡ��ѹmv
uint32_t ADC_Voltage(void)
{
	return (uint32_t)adc_data[5] * 3300 * 3 / 4095;
}

// GPIO
static void gpio_init(void)
{
	// PB2 ��ҡ�������Զ�����״̬
	rcu_periph_clock_enable(RCU_GPIOB);
	gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO_PIN_2);
}

// TIM2
static void tim2_init(void)
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
static void adc_dma_init(void)
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
static void adc_adc_init(void)
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

// DMA�ж�
void DMA_Channel0_IRQHandler(void)
{
	if (dma_interrupt_flag_get(DMA_CH0, DMA_INT_FLAG_FTF) == SET)
	{
		adc_achieve = 1;		// ��λ��־λ
		dma_interrupt_flag_clear(DMA_CH0, DMA_INT_FLAG_FTF);
	}
}


