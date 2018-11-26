/**
 * @File     : test.h
 * @Author   : Abdullah Younis
 *
 * NOTE: This test assumes an x86 architecture.
 */

#ifndef LIBFIPC_TEST_QUEUE_TEST
#define LIBFIPC_TEST_QUEUE_TEST

#include "queue.h"
#include "../libfipc_numa.h"

#define HALT		0
#define NULL_INVOCATION	1

// Test Variables
static uint64_t transactions = 10000;

static uint8_t producer_count = 1;
static uint8_t consumer_count = 1;

uint64_t batch_size = 1;

uint64_t mem_pool_order = 25;
uint64_t mem_pool_size;

#ifdef __KERNEL__
module_param( producer_count, byte, 0 );
module_param( consumer_count, byte, 0 );
#endif

static uint32_t* producer_cpus = NULL;
static uint32_t* consumer_cpus = NULL;
static int policy = 1;

#define pr_err printf

// Queue Variable
static queue_t queue;
static node_t**   node_tables = NULL;

// Thread Locks
static uint64_t completed_producers = 0;
static uint64_t completed_consumers = 0;
static uint64_t ready_consumers     = 0;
static uint64_t ready_producers     = 0;
static uint64_t test_ready          = 0;
static uint64_t test_finished       = 0;

// Time check
uint64_t global_start;
uint64_t global_end;
uint64_t cons_starts[32];
uint64_t cons_ends[32];
uint64_t prod_starts[32];
uint64_t prod_ends[32];

#endif
