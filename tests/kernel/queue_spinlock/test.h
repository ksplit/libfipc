/**
 * @File     : test.h
 * @Author   : Abdullah Younis
 *
 * NOTE: This test assumes an x86 architecture.
 */

#ifndef LIBFIPC_TEST_PING_PONG
#define LIBFIPC_TEST_PING_PONG

#include "../libfipc_test.h"
#include "queue.h"

// Queue Variable
static queue_t queue;

// Test Variables
static uint32_t transactions = 128;

static uint8_t producer_count = 2;
static uint8_t consumer_count = 2;

static uint8_t producer_cpus[32] = { 0, 1 };
static uint8_t consumer_cpus[32] = { 2, 3 };

// Request Types
#define HALT            0
#define NULL_INVOCATION 1

// Thread Locks
static uint64_t completed_producers;
static uint64_t completed_consumers;

#endif
