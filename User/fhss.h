/**********************
* ��Ƶ
**********************/
#ifndef __FHSS_H
#define __FHSS_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "gd32f1x0.h"


#define RF_CH_DEFAULT 	120	// Ĭ���ŵ�2520MHZ
#define FHSS_NUM				20	// ��ƵƵ����


uint8_t* fhss_update(void);
void fhss_hop(void);


#ifdef __cplusplus
}
#endif
#endif /*__FHSS_H */


