/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test.h                                                          **
 **  Description:   Minimal unit test framework for host and target                 **
 **  Last Modified:                                                                 **
 **  Licence:		GNU GPLv3                                                      **
 ************************************************************************************/

#ifndef __TEST_H
#define __TEST_H

#include <stdio.h>
#include <string.h>
#include <math.h>

/* Test result tracking */
extern int test_pass_count;
extern int test_fail_count;

/* Assertion macros */
#define TEST_ASSERT(cond) \
    do { \
        if (!(cond)) { \
            test_fail_count++; \
            printf("  FAIL: %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            return; \
        } \
    } while (0)

#define TEST_ASSERT_EQUAL(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            test_fail_count++; \
            printf("  FAIL: %s:%d: expected %d, got %d\n", __FILE__, __LINE__, (int)(expected), (int)(actual)); \
            return; \
        } \
    } while (0)

#define TEST_ASSERT_FLOAT_EQ(expected, actual, epsilon) \
    do { \
        float32_t diff = fabsf((float32_t)(expected) - (float32_t)(actual)); \
        if (diff > (epsilon)) { \
            test_fail_count++; \
            printf("  FAIL: %s:%d: expected %f, got %f (diff %f > %f)\n", __FILE__, __LINE__, \
                   (double)(expected), (double)(actual), (double)diff, (double)(epsilon)); \
            return; \
        } \
    } while (0)

#define TEST_ASSERT_NEAR(expected, actual, epsilon) \
    TEST_ASSERT_FLOAT_EQ(expected, actual, epsilon)

/* Test function type */
typedef void (*test_func_t)(void);

/* Test registration */
#define TEST_REGISTER(func) \
    do { \
        printf("Running %s...\n", #func); \
        func(); \
        test_pass_count++; \
    } while (0)

/* Test runner */
int test_main(void);

#endif /* __TEST_H */
