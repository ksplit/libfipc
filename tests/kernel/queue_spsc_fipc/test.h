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
static uint32_t transactions = 100000000;

static uint8_t producer_count = 1;
static uint8_t consumer_count = 1;

module_param( producer_count, byte, 0 );
module_param( consumer_count, byte, 0 );

// d820 
// NUMA node0 CPU(s):     0,4,8,12,16,20,24,28
// NUMA node1 CPU(s):     1,5,9,13,17,21,25,29
// NUMA node2 CPU(s):     2,6,10,14,18,22,26,30
// NUMA node3 CPU(s):     3,7,11,15,19,23,27,31

static uint8_t producer_cpus[32] = { 0, 8, 16, 24 };
static uint8_t consumer_cpus[32] = { 4, 12, 20, 28 };

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
