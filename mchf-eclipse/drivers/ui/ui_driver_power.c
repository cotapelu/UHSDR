/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     ui_driver_power.c                                               **
 **  Description:   Power management and configuration handling extracted from       **
 **                 ui_driver.c                                                      **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#include "ui_driver.h"
#include "ui_driver_power.h"
#include "ui_driver_utils.h"
#include "ui_lcd_hy28.h"
#include "ui_spectrum.h"
#include "audio_driver.h"
#include "codec.h"
#include "ui_configuration.h"
#include "soft_tcxo.h"
#include "adc.h"
#include "uhsdr_keypad.h"
#include "radio_management.h"

void UiDriver_CreateVoltageDisplay()
{
	// Create voltage
	UiLcdHy28_PrintTextCentered (ts.Layout->PWR_IND.x,ts.Layout->PWR_IND.y,ts.Layout->LEFTBOXES_IND.w,   "--.- V",  COL_PWR_IND,Black,0);
}

bool UiDriver_SaveConfiguration()
{
	bool savedConfiguration = true;

	const uint16_t scope_middle_y = sd.Slayout->full.h/2+sd.Slayout->full.y;

	const char* txp;
	uint16_t txc;

	switch (ts.configstore_in_use)
	{
	case CONFIGSTORE_IN_USE_FLASH:
		txp = "Saving settings to Flash Memory";
		break;
	case CONFIGSTORE_IN_USE_I2C:
		txp = "Saving settings to I2C EEPROM";
		break;
	default:
		txp = "Detected problems: Not saving";
		savedConfiguration = false;
	}
	UiLcdHy28_PrintTextCentered(sd.Slayout->full.x, scope_middle_y-6, sd.Slayout->full.w,txp,Blue,Black,0);

	if (savedConfiguration)
	{
		// save settings
		if (UiConfiguration_SaveEepromValues() == 0)
		{
			txp = "Saving settings finished";
			txc = Green;
		}
		else
		{
			txp = "Saving settings failed";
			txc = Red;
			savedConfiguration = false;
		}
		UiLcdHy28_PrintTextCentered(sd.Slayout->full.x, scope_middle_y+6, sd.Slayout->full.w,txp,txc,Black,0);
	}
	return savedConfiguration;
}


/*
 * @brief displays the visual information that power down is being executed and saves EEPROM if requested
 */
void UiDriver_PowerDownCleanup(bool saveConfiguration)
{
	const char* txp;
	// Power off all - high to disable main regulator

	ts.powering_down = 1;   // indicate that we should be powering down

	UiSpectrum_Clear();   // clear display under spectrum scope

	// hardware based mute
	Codec_MuteDAC(true);  // mute audio when powering down

	txp = " ";

	UiLcdHy28_PrintTextCentered(60,148,240,txp,Blue2,Black,0);
	UiLcdHy28_PrintTextCentered(60,156,240,"Powering off...",Blue2,Black,0);
	UiLcdHy28_PrintTextCentered(60,168,240,txp,Blue2,Black,0);

	if (saveConfiguration)
	{
		UiDriver_SaveConfiguration();
	}
	else
	{
		UiLcdHy28_PrintTextCentered(60,176,260,"...without saving settings...",Blue,Black,0);
	}


	if(saveConfiguration)
	{
		UiConfiguration_SaveEepromValues();     // save EEPROM values
	}

	HAL_Delay(3000);
}



/*
 * @brief Display external voltage
 */
void UiDriver_DisplayVoltage()
{
	uint32_t low_power_threshold = ((ts.low_power_config & LOW_POWER_THRESHOLD_MASK) + LOW_POWER_THRESHOLD_OFFSET) * 10;
	// did we detect a voltage change?

	uint32_t col = COL_PWR_IND;  // Assume normal voltage, so Set normal color

	if (pwmt.voltage < low_power_threshold + 50)
	{
		col = Red;
	}
	else if (pwmt.voltage < low_power_threshold + 100)
	{
		col = Orange;
	}
	else if (pwmt.voltage < low_power_threshold + 150)
	{
		col = Yellow;
	}

	static uint8_t voltage_blink = 0;
	// in case of low power shutdown coming, we let the voltage blink with 1hz
	if (pwmt.undervoltage_detected == true && voltage_blink < 1 )
	{
		col = Black;
	}
	voltage_blink++;
	if (voltage_blink == 2)
	{
		voltage_blink = 0;
	}

	char digits[6];
	snprintf(digits,6,"%2ld.%02ld",pwmt.voltage/100,pwmt.voltage%100);
	UiLcdHy28_PrintText(ts.Layout->PWR_IND.x,ts.Layout->PWR_IND.y,digits,col,Black,0);
}

/**
 * @brief Measures Voltage and controls undervoltage detection
 * @returns true if display update is required, false if not
 */
bool UiDriver_HandleVoltage()
{
	bool retval = false;
	// if this is set to true, we should update the display because something relevant for the user happened.

	// Collect samples
	if(pwmt.p_curr < POWER_SAMPLES_CNT)
	{
		// Add to accumulator
		pwmt.pwr_aver = pwmt.pwr_aver + HAL_ADC_GetValue(&hadc1);
		pwmt.p_curr++;
	}
	else
	{

		// Get average
		uint32_t val_p  = ((pwmt.pwr_aver/POWER_SAMPLES_CNT) * (ts.voltmeter_calibrate + 900))/2500;

		// Reset accumulator
		pwmt.p_curr     = 0;
		pwmt.pwr_aver   = 0;


		retval = pwmt.voltage != val_p;

		pwmt.voltage = val_p;


		uint32_t low_power_threshold = ((ts.low_power_config & LOW_POWER_THRESHOLD_MASK) + LOW_POWER_THRESHOLD_OFFSET) * 10;
		bool low_power_shutdown_enabled = (ts.low_power_config & LOW_POWER_ENABLE_MASK) == LOW_POWER_ENABLE;

		if (low_power_shutdown_enabled && (val_p < low_power_threshold ))
		{
			// okay, voltage is too low, we should indicate
			pwmt.undervoltage_detected = true;
			retval = true;

			if (ts.txrx_mode == TRX_MODE_RX)
			{
				if (ts.sysclock > ts.low_power_shutdown_time )         // only allow power-off in RX mode
				{
					UiDriver_PowerDownCleanup(true);
				}
			}
			else
			{
				ts.low_power_shutdown_time = ts.sysclock + LOW_POWER_SHUTDOWN_DELAY_TIME;
				// in tx mode, we extend the waiting time during the transmit, so that we don't switch off
				// right after a transmit but let the battery some time to "regenerate"
			}
		}
		else
		{
			if (pwmt.undervoltage_detected == true)
			{
				retval = true;
				pwmt.undervoltage_detected = false;
				Board_GreenLed(LED_STATE_ON);
			}
			ts.low_power_shutdown_time = ts.sysclock + LOW_POWER_SHUTDOWN_DELAY_TIME;
		}
	}

	return retval;
}

/**
 * @brief display measured temperature and current state of TCXO
 * @param temp in tenth of degrees Celsius (10 == 1 degree C)
 */
void UiDriver_DisplayTemperature(int temp)
{
	static int last_disp_temp = -100;
	uint32_t clr =  RadioManagement_TcxoGetMode() ==TCXO_ON ? Blue:Red;

	UiLcdHy28_PrintText(ts.Layout->TEMP_IND.x + TEMP_DATA,(ts.Layout->TEMP_IND.y + 1),"*",clr,Black,0);

	if (temp != last_disp_temp)
	{
		char out[10];
		char* txt_ptr;
		if((temp < 0) || (temp > 1000))  // is the temperature out of range?
		{
			txt_ptr = "RANGE!";
		}
		else {
			last_disp_temp = temp;

			int32_t ttemp = last_disp_temp;
			if(RadioManagement_TcxoIsFahrenheit())
			{
				ttemp = ((ttemp *9)/5) + 320;			// multiply by 1.8 and add 32 degrees
			}
			snprintf(out,10,"%3ld.%1ld",ttemp/10,(ttemp)%10);
			txt_ptr = out;
		}
		UiLcdHy28_PrintText(ts.Layout->TEMP_IND.x + TEMP_DATA + SMALL_FONT_WIDTH*1,(ts.Layout->TEMP_IND.y + 1),txt_ptr,Grey,Black,0);
	}
}

//*----------------------------------------------------------------------------
//* Function Name       : UiDriverHandleLoTemperature
//* Object              : display LO temperature and compensate drift
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
void UiDriver_HandleLoTemperature()
{
	if (SoftTcxo_HandleLoTemperatureDrift())
	{
		UiDriver_DisplayTemperature(lo.temp/1000); // precision is 0.1 represent by lowest digit
	}
}



//*----------------------------------------------------------------------------
//* Function Name       : UiDriverLoadSavedConfigurationAtStartup
//* Object              : Handles loading of saved configuration at startup
//* Input Parameters    :
//* Output Parameters   :
//* Functions called    :
//*----------------------------------------------------------------------------
bool UiDriver_LoadSavedConfigurationAtStartup()
{

	bool retval = false;
	CONFIG_DEFAULTS load_mode = CONFIG_DEFAULTS_KEEP;

	if (UiDriver_IsButtonPressed(BUTTON_F1_PRESSED) && UiDriver_IsButtonPressed(BUTTON_F3_PRESSED) && UiDriver_IsButtonPressed(BUTTON_F5_PRESSED))
	{
		load_mode = CONFIG_DEFAULTS_LOAD_ALL;
	}
	else if (UiDriver_IsButtonPressed(BUTTON_F2_PRESSED) && UiDriver_IsButtonPressed(BUTTON_F4_PRESSED))
	{
		load_mode = CONFIG_DEFAULTS_LOAD_FREQ;
	}

	if(load_mode != CONFIG_DEFAULTS_KEEP)
	{
		// let us make sure, the user knows what he/she is doing
		// in case of change of mindes, do normal configuration load

		uint32_t clr_fg = White, clr_bg = Black;
		const char* top_line = "";

		switch (load_mode)
		{
		case CONFIG_DEFAULTS_LOAD_ALL:
			clr_bg = Red;
			clr_fg = White;
			top_line = "ALL DEFAULTS";
			break;
		case CONFIG_DEFAULTS_LOAD_FREQ:
			clr_bg = Yellow;
			clr_fg = Black;
			top_line = "FREQ/MODE DEFAULTS";
			break;
		default:
			break;
		}


		UiLcdHy28_LcdClear(clr_bg);							// clear the screen
		// now do all of the warnings, blah, blah...
		UiLcdHy28_PrintTextCentered(2,05, 316, top_line,clr_fg,clr_bg,1);
		UiLcdHy28_PrintTextCentered(2,35, 316, "-> LOAD REQUEST <-",clr_fg,clr_bg,1);

		UiLcdHy28_PrintTextCentered(2,70, 316,
				"If you don't want to do this\n"
				"press POWER button to start normally.",clr_fg,clr_bg,0);

		UiLcdHy28_PrintTextCentered(2,120, 316,
				"If you want to load default settings\n"
				"press and hold BAND+ AND BAND-.\n"
				"Settings will be saved at POWEROFF",clr_fg,clr_bg,0);

		// On screen delay									// delay a bit...
		HAL_Delay(5000);

		// add this for emphasis
		UiLcdHy28_PrintTextCentered(2,195, 316,
				"Press BAND+ and BAND-\n"
				"to confirm loading",clr_fg,clr_bg,0);

		UiDriver_WaitForBandMAndBandPorPWR();

		const char* txp;

		if(UiDriver_IsButtonPressed(BUTTON_PWR_PRESSED))
		{
			clr_bg = Black;							// clear the screen
			clr_fg = White;
			txp = "...performing normal start...";

			load_mode = CONFIG_DEFAULTS_KEEP;
			retval = false;
		}
		else
		{
			txp = "...loading defaults in progress...";
			// call function to load values - default instead of EEPROM
			retval = true;
			ts.menu_var_changed = true;
		}
		UiLcdHy28_LcdClear(clr_bg);                         // clear the screen
		UiLcdHy28_PrintTextCentered(2,108,316,txp,clr_fg,clr_bg,0);
		HAL_Delay(5000);
	}

	bool load_freq_mode_defaults = false;
	bool load_eeprom_defaults = false;
	switch (load_mode)
	{
	case CONFIG_DEFAULTS_LOAD_ALL:
		load_eeprom_defaults = true;                           // yes, set flag to indicate that defaults will be loaded instead of those from EEPROM
		break;
	case CONFIG_DEFAULTS_LOAD_FREQ:
		load_freq_mode_defaults = true;
		break;
	default:
		break;
	}

	UiConfiguration_LoadEepromValues(load_freq_mode_defaults, load_eeprom_defaults);


	return retval;
}
