/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
**                                                                                 **
**                               mcHF QRP Transceiver                              **
**                             K Atanassov - M0NKA 2014                            **
**                                                                                 **
**---------------------------------------------------------------------------------**
**                                                                                 **
**  File name:                                                                     **
**  Description: Simple Timing Profiler + WCET Analysis                           **
**  Last Modified:                                                                 **
**  Licence:        GNU GPLv3                                                      **
************************************************************************************/

#ifndef __PROFILING_H
#define __PROFILING_H

typedef enum {
    ProfileAudioInterrupt = 0,
    ProfileTP1,
    ProfileTP2,
    ProfileTP3,
    ProfileTP4,
    ProfileTP5,
    ProfileTP6,
    ProfileTP7,
    ProfileTP8,
    ProfileTP9,
    ProfileFreeDV,
    FreeDVTXUnderrun,
    ProfileAudioISR_WCET,      /* WCET: Audio DMA ISR */
    ProfilePendSV_WCET,        /* WCET: PendSV Handler */
    ProfileMainLoop_WCET,      /* WCET: Main Loop iteration */
    ProfileSpectrum_WCET,      /* WCET: Spectrum/Waterfall update */
    ProfileMenu_WCET,          /* WCET: Menu rendering */
    EventProfileMax
} ProfiledEventNames;

typedef struct {
    uint32_t count;
    uint32_t start;
    uint32_t stop;
    uint64_t duration; // to get average divide duration by count
    uint32_t wcet;      /* Worst-case execution time (max single duration) */
} ProfilingTimedEvent;

typedef struct {
    ProfilingTimedEvent event[EventProfileMax];
} EventProfile_t;

extern EventProfile_t eventProfile;

#define PROFILE_EVENTS
inline void profileEvent(const ProfiledEventNames pe) {
#ifdef PROFILE_EVENTS
    if (pe<EventProfileMax) {
        eventProfile.event[pe].count++;
    }
#endif
}

/* WCET Analysis support */
#define WCET_BEGIN(pe) profileTimedEventStart(pe)
#define WCET_END(pe)   profileTimedEventStop(pe)

void profileEventsTracePrint();
void WCET_Report(void);

/***
 * How to use:
 * First start the cycle timer with profileTimeEventInit()
 * This resets the timer and makes it run.
 *
 * At any time call profileTimedEventStart(EventName)
 * to have the start being recorded
 * at the end of the event call profileTimedEventStop(EventName)
 * Duration of a single event should not be longer then 2^32 / clock frequency
 * i.e. ~25s @168 Mhz for a single event
 * total duration recorded should not be more than 2^64 cycles
 * which is quite a lot, i.e. there is no limit on that for the
 * average person in this universe
 *
 * Due to the approach multiple events can be recorded correctly but
 * outer events include the overhead of the calculation of the duration
 * only the innermost events are more or less accurate unless you time the profile functions and
 * remove the overhead later.
 */

// INLINE IMPLEMENTATIONS

#define DWT_CYCCNT    ((volatile uint32_t *)0xE0001004)
#define DWT_CONTROL   ((volatile uint32_t *)0xE0001000)
#define SCB_DEMCR     ((volatile uint32_t *)0xE000EDFC)
#define DWT_LAR       ((volatile uint32_t *)0xE0001FB0)

static inline void profileCycleCount_reset(){
    *SCB_DEMCR   |= 0x01000000;
#ifdef STM32F7
    *DWT_LAR = 0xC5ACCE55;                // <-- added unlock access to DWT (ITM, etc.)registers
#endif
    *DWT_CYCCNT  = 0; // reset the counter
    *DWT_CONTROL = 0;
}

static inline void profileCycleCount_start()
{
    *DWT_CONTROL = *DWT_CONTROL | 1;
}

static inline void profileCycleCount_stop()
{
    *DWT_CONTROL = *DWT_CONTROL  & ~1; //
}

static inline uint32_t profileCycleCount_get()
{
    return *DWT_CYCCNT;
}

static inline void profileTimedEventInit()
{
    profileCycleCount_reset();
    profileCycleCount_start();
}

static inline void profileTimedEventStart(const ProfiledEventNames pe)
{
#ifdef PROFILE_EVENTS
    if (pe<EventProfileMax) {
        eventProfile.event[pe].start = profileCycleCount_get();
    }
#endif

}
static inline void profileTimedEventStop(const ProfiledEventNames pe)
{

#ifdef PROFILE_EVENTS
    uint32_t stop = profileCycleCount_get();
    if (pe<EventProfileMax) {
        uint32_t duration = stop - eventProfile.event[pe].start;
        eventProfile.event[pe].stop = stop;
        eventProfile.event[pe].count++;
        eventProfile.event[pe].duration += duration;
        /* Update WCET if this run is longer */
        if (duration > eventProfile.event[pe].wcet) {
            eventProfile.event[pe].wcet = duration;
        }
    }
#endif

}
static inline void profileTimedEventReset(const ProfiledEventNames pe)
{
#ifdef PROFILE_EVENTS
    if (pe<EventProfileMax) {
        eventProfile.event[pe].start = 0;
        eventProfile.event[pe].stop = 0;
        eventProfile.event[pe].count = 0;
        eventProfile.event[pe].duration = 0;
        eventProfile.event[pe].wcet = 0;
    }
#endif
}

static inline  ProfilingTimedEvent* profileTimedEventGet(const ProfiledEventNames pe)
{
    ProfilingTimedEvent* pe_ptr = NULL;
#ifdef PROFILE_EVENTS
    if (pe<EventProfileMax) {
        pe_ptr = &eventProfile.event[pe];
    }
#endif
    return pe_ptr;
}

static inline void WCET_Report(void)
{
#ifdef PROFILE_EVENTS
    extern EventProfile_t eventProfile;
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
                case ProfileAudioISR_WCET:    budget_us = 100; break;  /* 100us budget */
                case ProfilePendSV_WCET:      budget_us = 500; break;  /* 500us budget */
                case ProfileMainLoop_WCET:    budget_us = 10000; break; /* 10ms budget */
                case ProfileSpectrum_WCET:    budget_us = 8000; break;  /* 8ms budget */
                case ProfileMenu_WCET:        budget_us = 5000; break;  /* 5ms budget */
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



#endif
