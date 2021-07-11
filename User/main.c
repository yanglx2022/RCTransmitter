/*****************************************************************************
* @brief   2.4Gң���� ֧���շ��������Ƶͨ��
* @author  yanglx2022
* @version V0.2
* @date    2021/7/11
******************************************************************************/
#include "delay.h"
#include "adc.h"
#include "led.h"
#include "bk2425.h"
#include "power.h"
#include "print.h"
#include "beep.h"
#include "fhss.h"

// ����ģʽ
typedef enum{
	MODE_NORMAL,	// ��������ģʽ
	MODE_MATCH,		// ��Է���ģʽ
	MODE_FHSS			// ������Ƶͼ��ģʽ
}WORK_MODE;
static WORK_MODE work_mode = MODE_NORMAL;

// Ψһid(��Ϊͨ�ŵ�ַ)
static uint32_t unique_id = 0;

// ʱ����ֵ
static const uint32_t CNT_1S = 1000000 / TIM_PERIOD_T2;

// ����
static uint32_t timeout_cnt = 0;	// ��ʱ����

// ����80% 60% 40% 20% 5%��Ӧ�ĵ�ѹ TODO��ʵ��ȷ��
static const uint32_t BAT_MV[5] = {5500, 5000, 4500, 4000, 3625};	// ��������6Vû��3.5V�ҵ������ѹ����

// ��Ƶͼ��
static uint8_t* p_fhss_list;

// Ĭ�ϵ�ַ
static const uint32_t ADDR_DEFAULT = 0x59414E47;

static void unique_id_get(uint32_t* id);
static void bk2425_error_exit(void);
static void mode_normal_enter(void);
static void mode_normal_proc(void);
static void mode_match_enter(void);
static void mode_match_proc(void);
static void mode_fhss_enter(void);
static void mode_fhss_proc(void);
static void timeout_proc(void);
static void power_key_proc(void);
static void battery_display(uint32_t voltage);

int main(void)
{
	// ����
	Power_On();
	// ��ȡunique_id
	unique_id_get(&unique_id);
	// ��ʼ��
	Led_Init();
	Led_Top(LED_ALL, LED_ALL);
	Print_Init();
	printf("����(id = 0x%08x)^_^\r\n", unique_id);
	if (BK2425_Init() == 0)
	{
		// ����ģ���ʼ��ʧ�����˳�
		bk2425_error_exit();
	}	
	work_mode = MODE_NORMAL;
	// 2s���ɿ������������뷢����Ƶͼ��ģʽ
	uint32_t start = delay_tick();
	while(!TICK_TIMEOUT(start, 4000000))
	{
		if (Power_Key() == 0)
		{
			mode_fhss_enter();
			break;
		}
	}
	// ����������ģʽ
	if (work_mode != MODE_FHSS)
	{
		mode_match_enter();
	}
	// ����ADCת��
	ADC_Init();
	// ���������ģʽ
	while(1)
	{
		if (adc_achieve)	// �������Ѳɼ�
		{
			adc_achieve = 0;
			switch(work_mode)
			{
				case MODE_MATCH:	// ��Է���ģʽ
					mode_match_proc();
					break;
				case MODE_FHSS:		// ������Ƶͼ��ģʽ
					mode_fhss_proc();
					break;
				case MODE_NORMAL:	// ��������ģʽ
					mode_normal_proc();
					break;
			}
			// ������ʾ
			battery_display(ADC_Voltage());
			// ��ʱ�ػ�
			timeout_proc();
			// �����ػ����ػ�
			power_key_proc();
		}
	}
}

// ��ȡΨһID
static void unique_id_get(uint32_t* id)
{
	uint32_t id1 = *(uint32_t*)(0x1FFFF7AC);
	uint32_t id2 = *(uint32_t*)(0x1FFFF7B0);
	uint32_t id3 = *(uint32_t*)(0x1FFFF7B4);
	*id = ((id1 >> 1) ^ (id2 >> 2) ^ (id3 >> 3));
}

// ����ģ������˳�
static void bk2425_error_exit(void)
{
	// ����LEDһ����˸(2Hz)10s
	printf("BK2425��ʼ��ʧ��,10s��ػ�...\r\n");
	for (int i = 0; i < 20; i++)
	{
		Led_Top(LED_ALL, LED_ALL);
		delay_ms(250);
		Led_Top(0, 0);
		delay_ms(250);
	}
	// �ػ�
	printf("�ټ�^_^\r\n");
	Power_Off();
}

// ������Է���ģʽ
static void mode_match_enter(void)
{
	printf("������Է���ģʽ\r\n");
	Beep(50);
	work_mode = MODE_MATCH;
	BK2425_TX_Mode(ADDR_DEFAULT);
	timeout_cnt = 0;
}

// ��Է���ģʽ����
static void mode_match_proc(void)
{
	// ����id
	if (BK2425_Transmit((uint8_t*)&unique_id, 4))
	{
		Beep(50);
		timeout_cnt = 0;
		// ת��������Ƶģʽ
		mode_fhss_enter();
		return;
	}
	else
	{
		timeout_cnt++;
	}
	// ����LEDͬʱ��ˮ��
	if (timeout_cnt < CNT_1S * 270)	// 4min30s
	{
		static uint8_t led_cnt = 0;
		Led_Top(LED_ALL, LED_0 << (led_cnt / 5));
		led_cnt++;
		if ((led_cnt / 5) > 2)
		{
			led_cnt = 0;
		}
	}
}

// ���뷢����Ƶͼ��ģʽ
static void mode_fhss_enter(void)
{
	printf("���뷢����Ƶͼ��ģʽ\r\n");
	work_mode = MODE_FHSS;
	p_fhss_list = fhss_update();
	BK2425_TX_Mode(unique_id);
}

// ������Ƶͼ��ģʽ����
static void mode_fhss_proc(void)
{
	if (BK2425_Transmit(p_fhss_list, FHSS_NUM))
	{
		// ת����������ģʽ
		mode_normal_enter();
		return;
	}
	timeout_cnt++;
	// ��ɫLED��ˮ��
	if (timeout_cnt < CNT_1S * 270)	// 4min30s
	{
		static uint8_t led_cnt = 0;
		Led_Top(LED_BLUE, LED_0 << (led_cnt / 5));
		led_cnt++;
		if ((led_cnt / 5) > 2)
		{
			led_cnt = 0;
		}
	}
}

// ������������ģʽ
static void mode_normal_enter(void)
{
	printf("������������ģʽ\r\n");
	work_mode = MODE_NORMAL;
	BK2425_TX_Mode(unique_id);
}

// ��������ģʽ����
static void mode_normal_proc(void)
{
	// ����ҡ�˰�������
	uint8_t data[10];
	ADC_Parse(data);
	// ��Ƶ
	fhss_hop();
	// ����
	if (BK2425_Transmit(data, 10))
	{
		if (timeout_cnt < CNT_1S * 270)	// 4min30s
		{
			Led_Top(LED_BLUE, LED_ALL);		// ���ͳɹ�����ɫLED��
		}
	}
	else
	{
		if (timeout_cnt < CNT_1S * 270)	// 4min30s
		{
			Led_Top(LED_RED, LED_ALL);		// ����ʧ���Ϻ�ɫLED��
		}
		static uint32_t failed_cnt = 0;	// ����ʧ�ܼ���
		failed_cnt++;
		// ��������ʧ��1s��ת��������Ƶģʽ
		if (failed_cnt >= CNT_1S)
		{
			failed_cnt = 0;
			mode_fhss_enter();
			return;
		}
	}
	// �ж�Ĭ��״̬
	if (data[1] ||  						// ��������
		*(uint32_t*)(data + 2) ||	// ��ҡ�˶���
		*(uint32_t*)(data + 6))		// ��ҡ�˶���
	{
		timeout_cnt = 0;
	}
	else
	{
		timeout_cnt++;
	}
}

// ��ʱ�ػ�����
static void timeout_proc(void)
{
	if (timeout_cnt >= CNT_1S * 270)	// 4min30s
	{
		// �ػ���ɫLED��˸30s
		if (timeout_cnt % (CNT_1S / 2) == 0)
		{
			Led_Top(LED_RED, LED_ALL);
		}
		else if (timeout_cnt % (CNT_1S / 4) == 0)
		{
			Led_Top(0, 0);
		}
		// �ػ�
		if (timeout_cnt >= CNT_1S * 300)	// 5min
		{
			printf("��ʱ5min�Զ��ػ�\r\n�ټ�^_^\r\n");
			Power_Off();
		}
	}
}

// ���ػ���
void power_key_proc(void)
{
	static uint32_t power_off_cnt = 0;
	static uint8_t power_off_enable = 0;
	if (Power_Key())
	{
		if (power_off_enable)	// ���⿪������ʱ������ִ����ػ�
		{
			power_off_cnt++;
			if (power_off_cnt >= CNT_1S)
			{
				Power_Off();
			}
		}
	}
	else
	{
		power_off_cnt = 0;
		power_off_enable = 1;
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




