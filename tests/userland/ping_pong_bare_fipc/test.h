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

static uint64_t transactions   = 10000000;

#define REQUESTER_CPU	0
#define RESPONDER_CPU	4
#define CHANNEL_ORDER	ilog2(sizeof(message_t)) + 7
#define QUEUE_DEPTH	1

// Thread Locks
pthread_mutex_t requester_mutex;
pthread_mutex_t responder_mutex;

#endif
