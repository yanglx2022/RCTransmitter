/**********************
* ��Ƶ
**********************/
#include "fhss.h"
#include "delay.h"
#include "stdlib.h"
#include "bk2425.h"

static uint8_t fhss_list[FHSS_NUM];	// ��Ƶͼ��
static uint8_t fhss_idx = 0;				// Ƶ������


// ������Ƶͼ��
uint8_t* fhss_update(void)
{
	// �����������
	uint32_t seed = delay_tick();
	srand(seed);
	// ��֤��Ƶ�������û���ظ����Ҳ�����Ĭ���ŵ�
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

// ��Ƶ
void fhss_hop(void)
{
	fhss_idx++;
	if (fhss_idx >= FHSS_NUM)
	{
		fhss_idx = 0;
	}
	BK2425_RF_CH(fhss_list[fhss_idx]);
}




