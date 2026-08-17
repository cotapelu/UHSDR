/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     ui_driver_utils.c                                               **
 **  Description:   Shared UI helper utilities extracted from ui_driver.c           **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#include "ui_driver.h"

int32_t change_and_limit_int(volatile int32_t val, int32_t change, int32_t min, int32_t max)
{
	val +=change;
	if (val< min)
	{
		val = min;
	}
	else if (val>  max)
	{
		val = max;
	}
	return val;
}


uint32_t change_and_limit_uint(volatile uint32_t val, int32_t change, uint32_t min, uint32_t max)
{
	if (change < 0 && ( -change  > (val - min)))
	{
		val = min;
	}
	else if (change > 0 && change >  max - val)
	{
		val = max;
	}
	else
	{
		val +=change;
	}
	return val;
}

uint32_t change_and_wrap_uint(volatile uint32_t val, int32_t change, uint32_t min, uint32_t max)
{
	if (change  > ((int32_t)max - (int32_t)val))
	{
		val = min;
	}
	else if ((change + (int32_t)val) <  (int32_t)min)
	{
		val = max;
	}
	else
	{
		val +=change;
	}
	return val;
}

void incr_wrap_uint8(volatile uint8_t* ptr, uint8_t min, uint8_t max )
{
	*ptr = (change_and_wrap_uint(*ptr,+1,min,max))&0xff;
}

void incr_wrap_uint16(volatile uint16_t* ptr, uint16_t min, uint16_t max )
{
	*ptr = (change_and_wrap_uint(*ptr,+1,min,max))&0xff;
}

void decr_wrap_uint8(volatile uint8_t* ptr, uint8_t min, uint8_t max )
{
	*ptr = (change_and_wrap_uint(*ptr,-1,min,max))&0xff;
}

void decr_wrap_uint16(volatile uint16_t* ptr, uint16_t min, uint16_t max )
{
	*ptr = (change_and_wrap_uint(*ptr,-1,min,max))&0xff;
}
