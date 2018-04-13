/**
 * @File     : test.h
 * @Author   : Abdullah Younis
 *
 * This test sends an empty message in a round trip
 * using the fipc library.
 *
 * NOTE: This test assumes an x86 architecture.
 */

#ifndef LIBFIPC_TEST_PING_PONG
#define LIBFIPC_TEST_PING_PONG

#include "../libfipc_test.h"

#define TRANSACTIONS	10000000
#define REQUESTER_CPU	0
#define RESPONDER_CPU	2
#define CHANNEL_ORDER	ilog2(sizeof(message_t)) + 7
#define MARSHALL_ORDER  4
#define BATCHED_ORDER   1

// Thread Locks
pthread_mutex_t requester_mutex;
pthread_mutex_t responder_mutex;

static uint64_t __attribute__ ((noinline)) null_invocation( void )
{
	return 0;
}

static uint64_t __attribute__ ((noinline)) increment( uint64_t a )
{
	return a + 1;
}

static uint64_t __attribute__ ((noinline)) add_2_nums( uint64_t a, uint64_t b )
{
	return a + b;
}

static uint64_t __attribute__ ((noinline)) add_3_nums( uint64_t a, uint64_t b, uint64_t c )
{
	return a + b + c;
}

static uint64_t __attribute__ ((noinline)) add_4_nums( uint64_t a, uint64_t b, uint64_t c, uint64_t d )
{
	return a + b + c + d;
}

static uint64_t __attribute__ ((noinline)) add_5_nums( uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e )
{
	return a + b + c + d + e;
}

static uint64_t __attribute__ ((noinline)) add_6_nums( uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e, uint64_t f )
{
	return a + b + c + d + e + f;
}

static uint64_t __attribute__ ((noinline)) add_7_nums( uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e, uint64_t f, uint64_t g )
{
	return a + b + c + d + e + f + g;
}

#endif
