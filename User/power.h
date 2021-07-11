/**********************
* ¿ª¹Ø»ú
**********************/
#ifndef __POWER_H
#define __POWER_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "gd32f1x0.h"


void Power_On(void);
void Power_Off(void);
uint8_t Power_Key(void);


#ifdef __cplusplus
}
#endif
#endif /*__POWER_H */


