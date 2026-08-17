#ifndef __ARM_MATH_H
#define __ARM_MATH_H
#include <stdint.h>
typedef float float32_t;
typedef struct { uint32_t numStages; float32_t *pCoeffs; float32_t *pState; } arm_biquad_casd_df1_inst_f32;
#endif
