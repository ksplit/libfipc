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

#define TRANSACTIONS	100000000
#define REQUESTER_CPU	0
#define RESPONDER_CPU	2
#define CHANNEL_ORDER	ilog2(sizeof(message_t)) + 7

// Thread Locks
static DECLARE_COMPLETION(requester_comp);
static DECLARE_COMPLETION(responder_comp);

#endif
