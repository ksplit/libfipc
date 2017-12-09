/**
 * @File     : test.h
 * @Author   : Abdullah Younis
 *
 * NOTE: This test assumes an x86 architecture.
 */

#ifndef LIBFIPC_TEST_QUEUE_TEST
#define LIBFIPC_TEST_QUEUE_TEST

#include "../libfipc_test.h"
#include "queue.h"

// Test Variables
static uint32_t transactions = 1000000;

static uint8_t producer_count = 4;
static uint8_t consumer_count = 4;

static uint8_t producer_cpus[32] = { 0, 8, 16, 24 };
static uint8_t consumer_cpus[32] = { 4, 12, 20, 28 };

// Queue Variables
static queue_t*** prod_queues = NULL;
static queue_t*** cons_queues = NULL;

// Request Types
#define HALT            2
#define NULL_INVOCATION 1

// Thread Locks
static uint64_t completed_producers = 0;
static uint64_t completed_consumers = 0;
static uint64_t ready_consumers     = 0;
static uint64_t ready_producers     = 0;
static uint64_t test_ready          = 0;
static uint64_t test_finished       = 0;

#endif
