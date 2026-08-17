/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                               mcHF QRP Transceiver                              **
 **                             K Atanassov - M0NKA 2014                            **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:                                                                     **
 **  Description:                                                                   **
 **  Last Modified:                                                                 **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

// Common
#include "uhsdr_board.h"
#include "profiling.h"

/*
 * In order to read the counters here, you'll need to connect
 * using a real-time debugger, pause execution and read values.
 * Not a big deal with ST-Link and Eclipse or gdb.
 */
EventProfile_t eventProfile;

#if 0
// the code below is only used to ease profiling with eclipse
// you just need hover over a variable to get the value
void dummy() {

    eventProfile.event[0].duration;
    eventProfile.event[0].count;
    eventProfile.event[1].duration;
    eventProfile.event[1].count;
    eventProfile.event[2].duration;
    eventProfile.event[2].count;
    eventProfile.event[3].duration;
    eventProfile.event[3].count;
    eventProfile.event[4].duration;
    eventProfile.event[4].count;
    eventProfile.event[5].duration;
    eventProfile.event[5].count;
    eventProfile.event[6].duration;
    eventProfile.event[6].count;
    eventProfile.event[7].duration;
    eventProfile.event[7].count;

    eventProfile.event[8].duration;
    eventProfile.event[8].count;


}
#endif


void profileEventsTracePrint()
{
#ifdef XPROFILE_EVENTS

            for (int i = 0;i < 10;i++)
            {
                ProfilingTimedEvent* ev_ptr = profileTimedEventGet(i);
                if (ev_ptr->count != 0)
                {
                    trace_printf("%d: %d uS per run\n",i, (ev_ptr->duration/(ev_ptr->count*168)));
                }
            }
#endif
}

#include <stdio.h>
#include "uhsdr_board.h"

void WCET_Report(void)
{
#ifdef PROFILE_EVENTS
    uint32_t cpu_hz = SystemCoreClock;
    
    printf("\n=== WCET Analysis Report ===\n");
    printf("CPU Frequency: %u Hz\n", cpu_hz);
    printf("%-30s %8s %8s %8s %8s\n", "Event", "Count", "Avg(us)", "WCET(us)", "Budget(us)");
    printf("--------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < EventProfileMax; i++) {
        ProfilingTimedEvent* ev = &eventProfile.event[i];
        if (ev->count > 0) {
            uint32_t avg_us = (ev->duration / ev->count) / (cpu_hz / 1000000);
            uint32_t wcet_us = ev->wcet / (cpu_hz / 1000000);
            uint32_t budget_us = 0;
            
            /* Define budgets for critical tasks */
            switch (i) {
                case ProfileAudioISR_WCET:    budget_us = 100; break;
                case ProfilePendSV_WCET:      budget_us = 500; break;
                case ProfileMainLoop_WCET:    budget_us = 10000; break;
                case ProfileSpectrum_WCET:    budget_us = 8000; break;
                case ProfileMenu_WCET:        budget_us = 5000; break;
                default:                      budget_us = 0; break;
            }
            
            printf("%-30s %8u %8u %8u %8u", 
                   (const char*[]){
                       "Audio ISR", "TP1", "TP2", "TP3", "TP4", "TP5",
                       "TP6", "TP7", "TP8", "TP9", "FreeDV", "TX Underrun",
                       "Audio ISR WCET", "PendSV WCET", "Main Loop WCET",
                       "Spectrum WCET", "Menu WCET"
                   }[i],
                   ev->count, avg_us, wcet_us, budget_us);
            
            if (budget_us > 0 && wcet_us > budget_us) {
                printf(" <-- OVER BUDGET!");
            }
            printf("\n");
        }
    }
    printf("================================================================================\n");
#endif
}
