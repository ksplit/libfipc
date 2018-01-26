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

module_param( producer_count, byte, 0 );
module_param( consumer_count, byte, 0 );

static uint8_t producer_cpus[32] = { 0, 8, 16, 24 };
static uint8_t consumer_cpus[32] = { 4, 12, 20, 28 };

// Queue Variables
static queue_t*** prod_queues_forw = NULL;
static queue_t*** cons_queues_forw = NULL;
static queue_t*** prod_queues_back = NULL;
static queue_t*** cons_queues_back = NULL;

static uint64_t* data = NULL;

// Request Types
#define HALT            0
#define NULL_INVOCATION 1
#define ADD_6_NUMS      2

// Thread Locks
static uint64_t completed_producers = 0;
static uint64_t completed_consumers = 0;
static uint64_t ready_consumers     = 0;
static uint64_t ready_producers     = 0;
static uint64_t test_ready          = 0;
static uint64_t test_finished       = 0;

#endif
