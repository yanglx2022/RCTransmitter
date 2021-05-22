/**********************
* BEEP
**********************/
#ifndef __BEEP_H
#define __BEEP_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "gd32f1x0.h"

void Beep_Init(void);
void Beep(int32_t time_ms);


#ifdef __cplusplus
}
#endif
#endif /*__BEEP_H */


