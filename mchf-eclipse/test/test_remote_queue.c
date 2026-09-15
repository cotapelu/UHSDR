/*  -*-  mode: c; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4; coding: utf-8  -*-  */
/************************************************************************************
 **                                                                                 **
 **                                        UHSDR                                    **
 **               a powerful firmware for STM32 based SDR transceivers              **
 **                                                                                 **
 **---------------------------------------------------------------------------------**
 **                                                                                 **
 **  File name:     test_remote_queue.c                                             **
 **  Description:   Host unit tests for remote command queue overflow behavior      **
 **                 (T46.2: drop-oldest on full queue)                              **
 **  Last Modified: 2026-08-28                                                      **
 **  Licence:       GNU GPLv3                                                      **
 ************************************************************************************/

#include "test.h"
#include <string.h>

/* -------------------------------------------------------------------------
 * Reimplementation of remote.c queue for host testing
 * (mirrors mchf-eclipse/drivers/remote/remote.c queue logic)
 * ------------------------------------------------------------------------- */
#define REMOTE_CMD_QUEUE_SIZE 8

typedef struct {
    uint8_t  type;
    int8_t   delta;
    uint8_t  button_id;
    uint8_t  pressed;
    uint32_t freq_hz;
} remote_cmd_t;

static remote_cmd_t remote_cmd_queue[REMOTE_CMD_QUEUE_SIZE];
static uint8_t remote_cmd_head = 0;
static uint8_t remote_cmd_tail = 0;
static uint8_t remote_cmd_count = 0;

static void Remote_Dequeue(remote_cmd_t *cmd);

static hal_status_t Remote_Enqueue(const remote_cmd_t *cmd)
{
    if (remote_cmd_count >= REMOTE_CMD_QUEUE_SIZE) {
        /* queue full — drop oldest */
        remote_cmd_t dummy;
        Remote_Dequeue(&dummy);
    }
    remote_cmd_queue[remote_cmd_head] = *cmd;
    remote_cmd_head = (remote_cmd_head + 1) % REMOTE_CMD_QUEUE_SIZE;
    remote_cmd_count++;
    return HAL_STATUS_OK;
}

static void Remote_Dequeue(remote_cmd_t *cmd)
{
    if (remote_cmd_count == 0) {
        cmd->type = 0xFF; /* empty sentinel */
        return;
    }
    *cmd = remote_cmd_queue[remote_cmd_tail];
    remote_cmd_tail = (remote_cmd_tail + 1) % REMOTE_CMD_QUEUE_SIZE;
    remote_cmd_count--;
}

static void queue_reset(void)
{
    memset(remote_cmd_queue, 0, sizeof(remote_cmd_queue));
    remote_cmd_head = 0;
    remote_cmd_tail = 0;
}

/* -------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */
static void test_queue_empty_initially(void)
{
    remote_cmd_t cmd;
    queue_reset();
    Remote_Dequeue(&cmd);
    TEST_ASSERT_EQUAL(0xFF, cmd.type); /* empty sentinel */
}

static void test_queue_enqueue_dequeue_single(void)
{
    remote_cmd_t cmd;
    queue_reset();
    remote_cmd_t in = {.type = 0x01, .delta = 5};
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, Remote_Enqueue(&in));
    Remote_Dequeue(&cmd);
    TEST_ASSERT_EQUAL(0x01, cmd.type);
    TEST_ASSERT_EQUAL(5, cmd.delta);
}

static void test_queue_enqueue_full_capacity(void)
{
    queue_reset();
    for (int i = 0; i < REMOTE_CMD_QUEUE_SIZE; i++) {
        remote_cmd_t in = {.type = (uint8_t)i, .delta = (int8_t)i};
        TEST_ASSERT_EQUAL(HAL_STATUS_OK, Remote_Enqueue(&in));
    }
    /* Queue should be full now; next enqueue drops oldest */
    remote_cmd_t extra = {.type = 0xFF, .delta = -1};
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, Remote_Enqueue(&extra));
}

static void test_queue_drain_after_full(void)
{
    remote_cmd_t cmd;
    queue_reset();
    /* Fill queue */
    for (int i = 0; i < REMOTE_CMD_QUEUE_SIZE; i++) {
        remote_cmd_t in = {.type = (uint8_t)i, .delta = (int8_t)i};
        Remote_Enqueue(&in);
    }
    /* Add one more — oldest (0) should be dropped */
    remote_cmd_t extra = {.type = 0xFE, .delta = -2};
    Remote_Enqueue(&extra);
    /* Drain all — should get items 1..7 then FE (8 items total) */
    for (int i = 1; i < REMOTE_CMD_QUEUE_SIZE; i++) {
        Remote_Dequeue(&cmd);
        TEST_ASSERT_EQUAL((uint8_t)i, cmd.type);
        TEST_ASSERT_EQUAL((int8_t)i, cmd.delta);
    }
    Remote_Dequeue(&cmd);
    TEST_ASSERT_EQUAL(0xFE, cmd.type);
    TEST_ASSERT_EQUAL(-2, cmd.delta);
    /* Queue should be empty now */
    Remote_Dequeue(&cmd);
    TEST_ASSERT_EQUAL(0xFF, cmd.type);
}

static void test_queue_wraparound(void)
{
    remote_cmd_t cmd;
    queue_reset();
    /* Fill queue */
    for (int i = 0; i < REMOTE_CMD_QUEUE_SIZE; i++) {
        remote_cmd_t in = {.type = (uint8_t)i, .delta = (int8_t)i};
        Remote_Enqueue(&in);
    }
    /* Drain 4 */
    for (int i = 0; i < 4; i++) {
        Remote_Dequeue(&cmd);
    }
    /* Add 4 more */
    for (int i = 0; i < 4; i++) {
        remote_cmd_t in = {.type = (uint8_t)(i + 100), .delta = (int8_t)(i + 100)};
        Remote_Enqueue(&in);
    }
    /* Drain all 8 and verify order */
    for (int i = 4; i < REMOTE_CMD_QUEUE_SIZE; i++) {
        Remote_Dequeue(&cmd);
        TEST_ASSERT_EQUAL((uint8_t)i, cmd.type);
    }
    for (int i = 0; i < 4; i++) {
        Remote_Dequeue(&cmd);
        TEST_ASSERT_EQUAL((uint8_t)(i + 100), cmd.type);
    }
}

static void test_queue_drop_oldest_behavior(void)
{
    remote_cmd_t cmd;
    queue_reset();
    /* Fill queue with 8 items */
    for (int i = 0; i < REMOTE_CMD_QUEUE_SIZE; i++) {
        remote_cmd_t in = {.type = (uint8_t)i, .delta = (int8_t)i};
        Remote_Enqueue(&in);
    }
    /* With drop-oldest: new item is accepted, oldest (type=0) is removed */
    remote_cmd_t extra = {.type = 0xFE, .delta = -2};
    hal_status_t st = Remote_Enqueue(&extra);
    TEST_ASSERT_EQUAL(HAL_STATUS_OK, st);
    /* Drain all 8 — should get items 1..7 then FE */
    for (int i = 1; i < REMOTE_CMD_QUEUE_SIZE; i++) {
        Remote_Dequeue(&cmd);
        TEST_ASSERT_EQUAL((uint8_t)i, cmd.type);
    }
    Remote_Dequeue(&cmd);
    TEST_ASSERT_EQUAL(0xFE, cmd.type);
    TEST_ASSERT_EQUAL(-2, cmd.delta);
}

/* -------------------------------------------------------------------------
 * Test registration
 * ------------------------------------------------------------------------- */
void test_remote_queue_register(void)
{
    printf("--- remote_queue (T46.2) ---\n");
    TEST_REGISTER(test_queue_empty_initially);
    TEST_REGISTER(test_queue_enqueue_dequeue_single);
    TEST_REGISTER(test_queue_enqueue_full_capacity);
    TEST_REGISTER(test_queue_drain_after_full);
    TEST_REGISTER(test_queue_wraparound);
    TEST_REGISTER(test_queue_drop_oldest_behavior);
    printf("\n");
}
