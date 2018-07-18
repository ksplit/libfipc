/**
 * @File     : test.h
 * @Author   : Abdullah Younis
 *
 * NOTE: This test assumes an x86 architecture.
 */

#ifndef LIBFIPC_TEST_QUEUE_TEST
#define LIBFIPC_TEST_QUEUE_TEST

#include "queue.h"

// Test Variables
static uint32_t transactions = 100000000;

static uint8_t producer_count = 1;
static uint8_t consumer_count = 1;

#ifdef __KERNEL__
module_param( producer_count, byte, 0 );
module_param( consumer_count, byte, 0 );
#endif

//d820 Numa node 0: 0,4,8,12,16,20,24,28
static uint8_t producer_cpus[32] = {  8, 0, 16, 24 };
static uint8_t consumer_cpus[32] = { 12, 4, 20, 28 };
#define pr_err printf

// Queue Variables
static queue_t*** prod_queues = NULL;
static queue_t*** cons_queues = NULL;
static node_t**   node_tables = NULL;

// Request Types
#define HALT            0
#define NULL_INVOCATION 1

// Thread Locks
static uint64_t completed_producers = 0;
static uint64_t completed_consumers = 0;
static uint64_t ready_consumers     = 0;
static uint64_t ready_producers     = 0;
static uint64_t test_ready          = 0;
static uint64_t test_finished       = 0;

#endif
