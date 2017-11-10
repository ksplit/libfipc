/**
 * @File     : test.h
 * @Author   : Abdullah Younis
 *
 * This test sends an (many) empty message(s) in a round trip
 * using the fipc library.
 *
 * NOTE: This test assumes an x86 architecture.
 */

#ifndef LIBFIPC_TEST_PING_PONG
#define LIBFIPC_TEST_PING_PONG

#include "../libfipc_test.h"

#define CHANNEL_ORDER ilog2(sizeof(message_t)) + 7

// Test Variables
static uint32_t transactions   = 1000000;
static uint32_t queue_depth    = 1;

module_param( transactions,     uint, 0 );
module_param( queue_depth,      uint, 0 );

static uint8_t requester_count = 2;
static uint8_t responder_count = 2;

static uint8_t requester_cpus[32] = { 0, 1 };
static uint8_t responder_cpus[32] = { 2, 3 };

// Thread Locks
static uint64_t completed_requesters = 0;
static uint64_t completed_responders = 0;
static uint64_t ready_requesters     = 0;
static uint64_t ready_responders     = 0;
static uint64_t test_ready           = 0;

#endif
