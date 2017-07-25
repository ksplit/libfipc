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
#include "perf_counter_helper.h"

#define TRANSACTIONS	10000000
#define REQUESTER_CPU	0	
#define RESPONDER_CPU	4
#define CHANNEL_ORDER	ilog2(sizeof(message_t)) + 7
#define BATCHED_ORDER   1

// Thread Locks
struct completion requester_comp;
struct completion responder_comp;

// Events
DECL_EVENT(e1);
DECL_EVENT(e2);
DECL_EVENT(e3);
DECL_EVENT(e4);
DECL_EVENT(e5);
DECL_EVENT(e6);
DECL_EVENT(e7);
DECL_EVENT(e8);

#endif
