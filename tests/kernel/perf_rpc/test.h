/**
 * @File     : test.h
 * @Author   : Abdullah Younis
 *
 * This test times an (many) rpc request(s) and response(s)
 * using the fipc library.
 *
 * The events can be programmed using the ev_idx and ev_msk parameters
 * Event ids and mask ids can be found in your cpu's architecture manual
 * Emulab's d710 (table 19-17, 19-19) and d820 (table 19-13, 19-15) machines can use this link:
 * https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3b-part-2-manual.html
 *
 * NOTE: This test assumes an x86 architecture.
 */

#ifndef LIBFIPC_TEST_PING_PONG
#define LIBFIPC_TEST_PING_PONG

#include "../libfipc_test.h"
#include "perf_counter_helper.h"

#define CHANNEL_ORDER ilog2(sizeof(message_t)) + 7

// Test Variables
static uint32_t transactions   = 1000000;
static uint8_t  requester_cpu  = 0;
static uint8_t  responder_cpu  = 1;
static uint32_t queue_depth    = 1;
static uint8_t  marshall_count = 0;

module_param( transactions,     uint, 0 );
module_param( requester_cpu,    byte, 0 );
module_param( responder_cpu,    byte, 0 );
module_param( queue_depth,      uint, 0 );
module_param( marshall_count,   byte, 0 );

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

// RPC Functions
static uint64_t noinline null_invocation( void )
{
	return 0;
}

static uint64_t noinline increment( uint64_t a )
{
	return a + 1;
}

static uint64_t noinline add_2_nums( uint64_t a, uint64_t b )
{
	return a + b;
}

static uint64_t noinline add_3_nums( uint64_t a, uint64_t b, uint64_t c )
{
	return a + b + c;
}

static uint64_t noinline add_4_nums( uint64_t a, uint64_t b, uint64_t c, uint64_t d )
{
	return a + b + c + d;
}

static uint64_t noinline add_5_nums( uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e )
{
	return a + b + c + d + e;
}

static uint64_t noinline add_6_nums( uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e, uint64_t f )
{
	return a + b + c + d + e + f;
}

static uint64_t noinline add_7_nums( uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e, uint64_t f, uint64_t g )
{
	return a + b + c + d + e + f + g;
}

#endif
