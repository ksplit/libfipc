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

#define TRANSACTIONS	100000
#define REQUESTER_CPU	0
#define RESPONDER_CPU	2
#define CHANNEL_ORDER	ilog2(sizeof(message_t)) + 7
#define BATCHED_ORDER   1

// Thread Locks
static DECLARE_COMPLETION(requester_comp);
static DECLARE_COMPLETION(responder_comp);

// Events
evt_sel_t *e1, *e2, *e3, *e4;

#endif