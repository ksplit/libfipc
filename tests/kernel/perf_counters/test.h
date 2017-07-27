/**
 * @File     : test.h
 * @Author   : Abdullah Younis
 *
 * This test sends establishes a requester thread and a responder thread on
 * seperate processers, and records certain cpu/cache events.
 *
 * The events can be programmed in main.c in the FILL_EVENT_OS macros
 * Event ids and mask ids can be found in your cpu's architecture manual
 * Emulab's d710 (table 19-17) and d820 (table 19-13, 19-15) machines can use this link:
 * https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3b-part-2-manual.html
 *
 * NOTE: This test assumes an x86 architecture.
 */

#ifndef LIBFIPC_TEST_PING_PONG
#define LIBFIPC_TEST_PING_PONG

#include "../libfipc_test.h"
#include "perf_counter_helper.h"

#define CHANNEL_ORDER	ilog2(sizeof(message_t)) + 7

// Test Variables
static uint32_t transactions  = 10000000;
static uint8_t  requester_cpu = 0;
static uint8_t  responder_cpu = 1;
static uint32_t queue_depth   = 1;

module_param( transactions,  uint, 0 );
module_param( requester_cpu, byte, 0 );
module_param( responder_cpu, byte, 0 );
module_param( queue_depth,   uint, 0 );

// Thread Locks
struct completion requester_comp;
struct completion responder_comp;

// Events
static evt_sel_t ev[8]     = { 0 };
static uint64_t  ev_val[8] = { 0 };

static uint32_t ev_num    = 0;
static uint8_t  ev_idx[8] = { 0 };
static uint8_t  ev_msk[8] = { 0 };

module_param_array( ev_idx, byte, &ev_num, 0 );
module_param_array( ev_msk, byte, NULL,    0 );

#endif
