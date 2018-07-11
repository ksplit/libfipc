#ifndef LIBFIPC_TEST_PING_PONG
#define LIBFIPC_TEST_PING_PONG

#include "../libfipc_test.h"
#include "queue.h"

// Queue Variable
static queue_t queue;

// Test Variables
static uint32_t transactions = 1000000;

static uint8_t producer_count = 4;
static uint8_t consumer_count = 4;
static uint8_t queue_count = 1;

static uint8_t producer_cpus[32] = { 0, 4, 8, 12 };
static uint8_t consumer_cpus[32] = { 16, 20, 24, 28 };

// Request Types
#define HALT            0
#define NULL_INVOCATION 1

// Thread Locks
static uint64_t completed_producers = 0;
static uint64_t completed_consumers = 0;
static uint64_t ready_consumers = 0;
static uint64_t ready_producers = 0;
static uint64_t test_ready = 0;
static uint64_t test_finished = 0;

#endif