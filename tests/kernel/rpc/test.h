/**
 * @File     : test.h
 * @Author   : Abdullah Younis
 *
 * This test times  rpc request(s) and response(s) throughput using
 * the fipc library.
 *
 * NOTE: This test assumes an x86 architecture.
 */

#ifndef LIBFIPC_TEST_RPC
#define LIBFIPC_TEST_RPC

#include "../libfipc_test.h"

#define CHANNEL_ORDER ilog2(sizeof(message_t)) + 12

// Test Variables
static uint32_t transactions   = 1000;
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
