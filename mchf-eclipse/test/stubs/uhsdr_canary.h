#ifndef __UHSDR_CANARY_H
#define __UHSDR_CANARY_H
#include "uhsdr_types.h"
void     Canary_Create(void);
bool     Canary_IsIntact(void);
uint8_t* Canary_GetAddr(void);
#endif
