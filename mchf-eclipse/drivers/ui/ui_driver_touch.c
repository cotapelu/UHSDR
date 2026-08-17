/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     ui_driver_touch.c                                               **
 **  Description:   Touchscreen handling extracted from ui_driver.c                 **
 **  Last Modified:                                                                 **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "ui_driver.h"
#include "ui_driver_touch.h"
#include "ui_driver_utils.h"
#include "uhsdr_keypad.h"

#define CrossCheckCount 3
//cross size definitions, must be odd
#define CrossSizeH 11
#define CrossSizeV 11
static void DrawCross(int16_t* coord,uint16_t color)
{
	UiLcdHy28_DrawStraightLine(coord[0]-(CrossSizeH/2), coord[1],CrossSizeH,        LCD_DIR_HORIZONTAL,color);
	UiLcdHy28_DrawStraightLine(coord[0], coord[1]-(CrossSizeV/2),CrossSizeV,        LCD_DIR_VERTICAL,color);
}


bool UiDriver_CheckTouchRegion(const UiArea_t* tr_p)
{
	return ((ts.tp->hr_x <= (tr_p->x+tr_p->w)) &&
				(ts.tp->hr_x >= (tr_p->x)) &&
				(ts.tp->hr_y <= (tr_p->y+tr_p->h))) &&
				(ts.tp->hr_y >= (tr_p->y));

}

bool is_touchscreen_pressed()
{
	return (ts.tp->state == TP_DATASETS_VALID);	// touchscreen data available
}

bool UiDriver_ProcessTouchActions(const touchaction_list_descr_t* tld, bool is_long_press)
{
	bool retval = false;
	if (tld != NULL)
	{
		for (uint32_t idx = 0; idx < tld->size; idx++)
		{
			if (UiDriver_CheckTouchRegion(&tld->actions[idx].region))
			{
			    if (is_long_press)
			    {
			        if (tld->actions[idx].function_long_press != NULL)
			        {
			            (*tld->actions[idx].function_long_press)();
			        }
			    }
			    else
			    {
			        if (tld->actions[idx].function_short_press != NULL)
			        {
			            (*tld->actions[idx].function_short_press)();
			        }
			    }
			    retval = true;
			    break;
			}
		}
	}
	return retval;
}

void UiDriver_TouchscreenCalibrationRun()
{
    UiLcdHy28_TouchscreenReadCoordinates();
    ts.tp->state = TP_DATASETS_NONE;
    uint16_t MAX_X=ts.Layout->Size.x; uint16_t MAX_Y=ts.Layout->Size.y;

    int16_t cross[5][4] =
    {
            {      20,      20,0,0},
            {MAX_X-20,      20,0,0},
            {      20,MAX_Y-20,0,0},
            {MAX_X-20,MAX_Y-20,0,0},
            { MAX_X/2, MAX_Y/2,0,0},
    };

    //reset calibration coefficients before acquiring points
    for(int16_t m=0; m<6; m++)
    {
        ts.tp->cal[m]=0;
    }

    ts.tp->cal[0]=65536;
    ts.tp->cal[4]=65536;

    for (int16_t idx = 0; idx < 5; idx++)
    {
        UiDriver_DoCrossCheck(cross[idx]);
    }

    //calibration algorithm based on publication:
    //"Calibration in touch-screen systems" Texas Instruments
    //Analog Applications Journal 3Q 2007

    /*//test vectors
    int16_t cross[0][4] = {     128,     384,1698,2258};
    int16_t cross[1][4] = {      64,     192, 767,1149};
    int16_t cross[2][4] = {     192,     192,2807,1327};
    int16_t cross[3][4] = {     192,     576,2629,3367};
    int16_t cross[4][4] = {      64,     576, 588,3189};*/

    //matrices field definitions
    float mA[3*5];
    float mAT[3*5];
    float mATAinv[3*3];
    float mbuff[3*3];
    float mcom[3*5];
    float mX[5];
    float mY[5];
    float mABC[3];
    float mDEF[3];

    //matrix data init
    for (int m=0; m < 5; m++)
    {
        mA[3*m+0]=cross[m][2];
        mA[3*m+1]=cross[m][3];
        mA[3*m+2]=1.0;
        mX[m]= cross[m][0];
        mY[m]= cross[m][1];
    }

    //create matrices instances
    arm_matrix_instance_f32 m_A,m_AT,m_ATAinv,m_X,m_Y,m_ABC,m_DEF,m_buff,m_com;

    //init of matrices
    arm_mat_init_f32(&m_A,5,3,mA);
    arm_mat_init_f32(&m_AT,3,5,mAT);
    arm_mat_init_f32(&m_ATAinv,3,3,mATAinv);
    arm_mat_init_f32(&m_X,5,1,mX);
    arm_mat_init_f32(&m_Y,5,1,mY);
    arm_mat_init_f32(&m_ABC,3,1,mABC);
    arm_mat_init_f32(&m_DEF,3,1,mDEF);
    arm_mat_init_f32(&m_buff,3,3,mbuff);
    arm_mat_init_f32(&m_com,3,5,mcom);

    //real computation
    arm_mat_trans_f32(&m_A,&m_AT);           //A^T           size 5x3 -> 3x5
    arm_mat_mult_f32(&m_AT,&m_A,&m_buff);        //A^T x A   size 3x5 * 5x3 -> 3x3
    arm_mat_inverse_f32(&m_buff,&m_ATAinv);  //(A^T x A)^-1  size 3x3
    arm_mat_mult_f32(&m_ATAinv,&m_AT,&m_com);//(A^T x A)^-1 x A^T   m_com is common matrix for estimating coefficients for X and Y      size 3x3 * 3x5 -> 3x5

    arm_mat_mult_f32(&m_com,&m_X,&m_ABC);   //calculating the coefficients for X data    size 3x5 * 5x1  -> 3x1
    arm_mat_mult_f32(&m_com,&m_Y,&m_DEF);   //calculating the coefficients for Y data    size 3x5 * 5x1  -> 3x1

    //store cal parameters
    for (int m=0; m < 3; m++)
    {
        ts.tp->cal[m]=mABC[m]*65536;
        ts.tp->cal[m+3]=mDEF[m]*65536;
    }
}

bool UiDriver_TouchscreenCalibration()
{
	bool retval = false;
	uint16_t MAX_X=ts.Layout->Size.x; uint16_t MAX_Y=ts.Layout->Size.y;

    bool run_calibration = false;

    const uint32_t clr_bg = Black;
    const uint32_t clr_fg = White;

    Keypad_Scan();

    //if (UiDriver_IsButtonPressed(TOUCHSCREEN_ACTIVE) && UiDriver_IsButtonPressed(BUTTON_F5_PRESSED))
    if (UiDriver_IsButtonPressed(TOUCHSCREEN_ACTIVE))
    {
        //wait for a moment to filter out some unwanted spikes
        HAL_Delay(500);
        Keypad_Scan();

        if(UiDriver_IsButtonPressed(TOUCHSCREEN_ACTIVE))
        {

            UiLcdHy28_LcdClear(clr_bg);

            if (ts.tp->present)
            {
                // now do all of the warnings, blah, blah...
                UiLcdHy28_PrintTextCentered(2,05,MAX_X-4,"TOUCH CALIBRATION",clr_fg,clr_bg,1);
                UiLcdHy28_PrintTextCentered(2, 70, MAX_X-4, "If you don't want to do this\n"
                        "press POWER button to start normally.\n"
                        " Settings will be saved at POWEROFF"
                        ,clr_fg,clr_bg,0);

                // delay a bit...
                HAL_Delay(3000);

                // add this for emphasis
                UiLcdHy28_PrintTextCentered(2, 195, MAX_X-4, "Press BAND+ and BAND-\n"
                        "to start calibration",clr_fg,clr_bg,0);

                UiDriver_WaitForBandMAndBandPorPWR();

                if (UiDriver_IsButtonPressed(BUTTON_PWR_PRESSED))
                {
                    UiLcdHy28_LcdClear(Black);							// clear the screen
                    UiLcdHy28_PrintTextCentered(2,108,MAX_X-4,"      ...performing normal start...",White,Black,0);
                    HAL_Delay(3000);
                }
                else
                {
                    run_calibration = true;
                }
            }
            else
            {
                UiLcdHy28_PrintTextCentered(2,05,MAX_X-4,"TOUCHSCREEN ERROR",clr_fg,clr_bg,1);
                UiLcdHy28_PrintTextCentered(2, 70, MAX_X-4, "A touchscreen press was detected\n"
                        "but no touchscreen controller found\n"
                        "Calibration cannot be executed!"
                        ,clr_fg,clr_bg,0);
                // delay a bit...
                HAL_Delay(3000);
            }
		}
	}

	if (run_calibration)
	{
	    UiLcdHy28_LcdClear(clr_bg);
	    UiLcdHy28_PrintTextCentered(2,70, MAX_X-4,
	            "On the next screen crosses will appear.\n"
	            "Touch as exact as you can on the middle\n"
	            "of each cross. After three valid\n"
	            "samples position of cross changes.\n"
	            "Repeat until the five test positions\n"
	            "are finished.",clr_fg,clr_bg,0);

	    UiLcdHy28_PrintTextCentered(2,195,MAX_X-4,"Touch at any position to start.",clr_fg,clr_bg,0);

 	    UiDriver_WaitForButtonPressed(TOUCHSCREEN_ACTIVE);

	    UiLcdHy28_LcdClear(clr_bg);
	    UiLcdHy28_PrintTextCentered(2,100,MAX_X-4,"Wait one moment please...",Yellow,clr_bg,0);
	    HAL_Delay(1000);

	    UiDriver_TouchscreenCalibrationRun();

	    UiLcdHy28_LcdClear(clr_bg);

	    HAL_Delay(2000);
	    retval = true;
	    ts.menu_var_changed = true;
	}
	return retval;
}

void UiDriver_DoCrossCheck(int16_t cross[])
{
	uint16_t MAX_X=ts.Layout->Size.x;
	uint32_t clr_fg, clr_bg;
	clr_bg = Black;
	clr_fg = White;

	UiLcdHy28_LcdClear(clr_bg);
	DrawCross(cross,clr_fg);

	char txt_buf[40];
	uchar datavalid = 0, samples = 0;

	int16_t* xt_corr=&cross[2];
	int16_t* yt_corr=&cross[3];

	*xt_corr=0;
	*yt_corr=0;

	do
	{
	    UiDriver_WaitForButtonPressed(TOUCHSCREEN_ACTIVE);

		if (UiLcdHy28_TouchscreenHasProcessableCoordinates())
		{
			//if(abs(ts.tp->hr_x - cross[0]) < MaxTouchError && abs(ts.tp->hr_y - cross[1]) < MaxTouchError)
			//{
				datavalid++;
				*xt_corr += ts.tp->hr_x;
				*yt_corr += ts.tp->hr_y;
				clr_fg = Green;
				snprintf(txt_buf,40,"Try (%d) error: x = %+d / y = %+d",datavalid,ts.tp->hr_x-cross[0],ts.tp->hr_y-cross[1]);	//show misajustments
			/*}
			else
			{
				clr_fg = Red;
				snprintf(txt_buf,40,"Try (%d) BIG error: x = %+d / y = %+d",samples,ts.tp->hr_x-cross[0],ts.tp->hr_y-cross[1]);	//show misajustments
			}*/
			samples++;
			UiLcdHy28_PrintTextCentered(2,70,MAX_X-4,txt_buf,clr_fg,clr_bg,0);

			snprintf(txt_buf,40,"RAW: x = %+d / y = %+d",ts.tp->xraw,ts.tp->yraw);	//show misajustments
			UiLcdHy28_PrintTextCentered(2,85,MAX_X-4,txt_buf,clr_fg,clr_bg,0);
			ts.tp->state = TP_DATASETS_PROCESSED;
		}
	}
	while(datavalid < CrossCheckCount);

	UiLcdHy28_PrintTextCentered(2,100,MAX_X-4,"Wait one moment please...",Yellow,clr_bg,0);

	*xt_corr/=CrossCheckCount; //average the data
	*yt_corr/=CrossCheckCount;

	HAL_Delay(2000);
}

void UiDriver_HandleTouchScreen(bool is_long_press)
{
	if(is_touchscreen_pressed())
	{
		uint32_t touchaction_idx = ts.menu_mode == true?1:0;

		if (ts.show_debug_info)					// show coordinates for coding purposes
		{
			char text[14];
			snprintf(text,14,"%04d%s%04d%s",ts.tp->hr_x," : ",ts.tp->hr_y,"  ");

    

			UiLcdHy28_PrintText(0,ts.Layout->LOADANDDEBUG_Y,text,White,Black,0);
		}

		bool TouchProcessed=0;
		if(ts.SpectrumResize_flag==true
				&& ts.menu_mode==0)
		{
			TouchProcessed=UiDriver_Process_WFscope_RatioChange();
		}
		else if(ts.VirtualKeysShown_flag)
		{
			TouchProcessed=UiVk_Process_VirtualKeypad(is_long_press);
		}

		if(!TouchProcessed)
		{
			UiDriver_ProcessTouchActions(&ts.Layout->touchaction_list[touchaction_idx], is_long_press);
		}

		ts.tp->state = TP_DATASETS_PROCESSED;							// set statemachine to data fetched
	}
}

void UiDriver_HandleTouchScreenShortPress(bool is_long_press)
{
    UiDriver_HandleTouchScreen(false);
}

void UiDriver_HandleTouchScreenLongPress(bool is_long_press)
{
    UiDriver_HandleTouchScreen(true);
}

