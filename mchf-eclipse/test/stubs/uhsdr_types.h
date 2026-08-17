#ifndef __UHSDR_TYPES_H
#define __UHSDR_TYPES_H
#include <stdint.h>
#include "arm_math.h"
typedef uint8_t uchar; typedef uint16_t ushort; typedef uint32_t uint; typedef uint32_t ulong; typedef int32_t long32;
typedef int bool;
#define true 1
#define false 0
typedef struct { float32_t i; float32_t q; } iq_float_t;
typedef enum { IQ_TRANS_OFF = 0, IQ_TRANS_ON, IQ_TRANS_NUM } iq_trans_idx_t;
#endif
