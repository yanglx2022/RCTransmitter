/**********************
* 跳频
**********************/
#ifndef __FHSS_H
#define __FHSS_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "gd32f1x0.h"


#define RF_CH_DEFAULT 	120	// 默认信道2520MHZ
#define FHSS_NUM				20	// 跳频频点数


uint8_t* fhss_update(void);
void fhss_hop(void);


#ifdef __cplusplus
}
#endif
#endif /*__FHSS_H */


