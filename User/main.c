/*****************************************************************************
* @brief   2.4G遥控器 支持收发配对与跳频通信
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

// 工作模式
typedef enum{
	MODE_NORMAL,	// 正常发送模式
	MODE_MATCH,		// 配对发送模式
	MODE_FHSS			// 发送跳频图案模式
}WORK_MODE;
static WORK_MODE work_mode = MODE_NORMAL;

// 唯一id(作为通信地址)
static uint32_t unique_id = 0;

// 时间阈值
static const uint32_t CNT_1S = 1000000 / TIM_PERIOD_T2;

// 计数
static uint32_t timeout_cnt = 0;	// 超时计数

// 电量80% 60% 40% 20% 5%对应的电压 TODO待实测确认
static const uint32_t BAT_MV[5] = {5500, 5000, 4500, 4000, 3625};	// 假设满电6V没电3.5V且电量与电压线性

// 跳频图案
static uint8_t* p_fhss_list;

// 默认地址
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
	// 开机
	Power_On();
	// 获取unique_id
	unique_id_get(&unique_id);
	// 初始化
	Led_Init();
	Led_Top(LED_ALL, LED_ALL);
	Print_Init();
	printf("开机(id = 0x%08x)^_^\r\n", unique_id);
	if (BK2425_Init() == 0)
	{
		// 无线模块初始化失败则退出
		bk2425_error_exit();
	}	
	work_mode = MODE_NORMAL;
	// 2s内松开开机按键进入发送跳频图案模式
	uint32_t start = delay_tick();
	while(!TICK_TIMEOUT(start, 4000000))
	{
		if (Power_Key() == 0)
		{
			mode_fhss_enter();
			break;
		}
	}
	// 否则进入配对模式
	if (work_mode != MODE_FHSS)
	{
		mode_match_enter();
	}
	// 启动ADC转换
	ADC_Init();
	// 处理各工作模式
	while(1)
	{
		if (adc_achieve)	// 新数据已采集
		{
			adc_achieve = 0;
			switch(work_mode)
			{
				case MODE_MATCH:	// 配对发送模式
					mode_match_proc();
					break;
				case MODE_FHSS:		// 发送跳频图案模式
					mode_fhss_proc();
					break;
				case MODE_NORMAL:	// 正常发送模式
					mode_normal_proc();
					break;
			}
			// 电量显示
			battery_display(ADC_Voltage());
			// 超时关机
			timeout_proc();
			// 长按关机键关机
			power_key_proc();
		}
	}
}

// 获取唯一ID
static void unique_id_get(uint32_t* id)
{
	uint32_t id1 = *(uint32_t*)(0x1FFFF7AC);
	uint32_t id2 = *(uint32_t*)(0x1FFFF7B0);
	uint32_t id3 = *(uint32_t*)(0x1FFFF7B4);
	*id = ((id1 >> 1) ^ (id2 >> 2) ^ (id3 >> 3));
}

// 无线模块错误退出
static void bk2425_error_exit(void)
{
	// 红蓝LED一起闪烁(2Hz)10s
	printf("BK2425初始化失败,10s后关机...\r\n");
	for (int i = 0; i < 20; i++)
	{
		Led_Top(LED_ALL, LED_ALL);
		delay_ms(250);
		Led_Top(0, 0);
		delay_ms(250);
	}
	// 关机
	printf("再见^_^\r\n");
	Power_Off();
}

// 进入配对发送模式
static void mode_match_enter(void)
{
	printf("进入配对发送模式\r\n");
	Beep(50);
	work_mode = MODE_MATCH;
	BK2425_TX_Mode(ADDR_DEFAULT);
	timeout_cnt = 0;
}

// 配对发送模式处理
static void mode_match_proc(void)
{
	// 发送id
	if (BK2425_Transmit((uint8_t*)&unique_id, 4))
	{
		Beep(50);
		timeout_cnt = 0;
		// 转到发送跳频模式
		mode_fhss_enter();
		return;
	}
	else
	{
		timeout_cnt++;
	}
	// 红蓝LED同时流水灯
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

// 进入发送跳频图案模式
static void mode_fhss_enter(void)
{
	printf("进入发送跳频图案模式\r\n");
	work_mode = MODE_FHSS;
	p_fhss_list = fhss_update();
	BK2425_TX_Mode(unique_id);
}

// 发送跳频图案模式处理
static void mode_fhss_proc(void)
{
	if (BK2425_Transmit(p_fhss_list, FHSS_NUM))
	{
		// 转到正常发送模式
		mode_normal_enter();
		return;
	}
	timeout_cnt++;
	// 蓝色LED流水灯
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

// 进入正常发送模式
static void mode_normal_enter(void)
{
	printf("进入正常发送模式\r\n");
	work_mode = MODE_NORMAL;
	BK2425_TX_Mode(unique_id);
}

// 正常发送模式处理
static void mode_normal_proc(void)
{
	// 解析摇杆按键数据
	uint8_t data[10];
	ADC_Parse(data);
	// 跳频
	fhss_hop();
	// 发送
	if (BK2425_Transmit(data, 10))
	{
		if (timeout_cnt < CNT_1S * 270)	// 4min30s
		{
			Led_Top(LED_BLUE, LED_ALL);		// 发送成功上蓝色LED亮
		}
	}
	else
	{
		if (timeout_cnt < CNT_1S * 270)	// 4min30s
		{
			Led_Top(LED_RED, LED_ALL);		// 发送失败上红色LED亮
		}
		static uint32_t failed_cnt = 0;	// 发送失败计数
		failed_cnt++;
		// 连续发送失败1s则转到发送跳频模式
		if (failed_cnt >= CNT_1S)
		{
			failed_cnt = 0;
			mode_fhss_enter();
			return;
		}
	}
	// 判断默认状态
	if (data[1] ||  						// 按键按下
		*(uint32_t*)(data + 2) ||	// 左摇杆动作
		*(uint32_t*)(data + 6))		// 右摇杆动作
	{
		timeout_cnt = 0;
	}
	else
	{
		timeout_cnt++;
	}
}

// 超时关机处理
static void timeout_proc(void)
{
	if (timeout_cnt >= CNT_1S * 270)	// 4min30s
	{
		// 关机红色LED闪烁30s
		if (timeout_cnt % (CNT_1S / 2) == 0)
		{
			Led_Top(LED_RED, LED_ALL);
		}
		else if (timeout_cnt % (CNT_1S / 4) == 0)
		{
			Led_Top(0, 0);
		}
		// 关机
		if (timeout_cnt >= CNT_1S * 300)	// 5min
		{
			printf("超时5min自动关机\r\n再见^_^\r\n");
			Power_Off();
		}
	}
}

// 检测关机键
void power_key_proc(void)
{
	static uint32_t power_off_cnt = 0;
	static uint8_t power_off_enable = 0;
	if (Power_Key())
	{
		if (power_off_enable)	// 避免开机按键时间过长又触发关机
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

// 根据电压显示电量
static void battery_display(uint32_t voltage)
{
	if (voltage >= BAT_MV[0])				// 80% 4个蓝LED
	{
		Led_Bottom(LED_BLUE, LED_ALL);
	}
	else if (voltage >= BAT_MV[1])	// 60% 3个蓝LED
	{
		Led_Bottom(LED_BLUE, LED_0 | LED_1 | LED_2);
	}
	else if (voltage >= BAT_MV[2])	// 40% 2个蓝LED
	{
		Led_Bottom(LED_BLUE, LED_0 | LED_1);
	}
	else if (voltage >= BAT_MV[3])	// 20% 1个蓝LED
	{
		Led_Bottom(LED_BLUE, LED_0);
	}
	else if (voltage >= BAT_MV[4])	// 5% 1个红LED
	{
		Led_Bottom(LED_RED, LED_1);
	}
	else														// <5% 下部所有红LED闪烁
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




