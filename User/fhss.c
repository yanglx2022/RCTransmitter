/**********************
* 跳频
**********************/
#include "fhss.h"
#include "delay.h"
#include "stdlib.h"
#include "bk2425.h"

static uint8_t fhss_list[FHSS_NUM];	// 跳频图案
static uint8_t fhss_idx = 0;				// 频道索引


// 更新跳频图案
uint8_t* fhss_update(void)
{
	// 更新随机种子
	uint32_t seed = delay_tick();
	srand(seed);
	// 保证跳频随机序列没有重复项且不包括默认信道
	fhss_list[0] = rand() % 127;
	for(int i = 1; i < FHSS_NUM; i++)
	{
		while(1)
		{
			uint8_t fhss = rand() % 127;
			uint8_t equal = 0;
			for(int j = 0; j < i; j++)
			{
				if (fhss == fhss_list[j] || fhss == RF_CH_DEFAULT)
				{
					equal = 1;
					break;
				}
			}
			if (equal == 0)
			{
				fhss_list[i] = fhss;
				break;
			}
		}
	}
	return fhss_list;
}

// 跳频
void fhss_hop(void)
{
	fhss_idx++;
	if (fhss_idx >= FHSS_NUM)
	{
		fhss_idx = 0;
	}
	BK2425_RF_CH(fhss_list[fhss_idx]);
}




