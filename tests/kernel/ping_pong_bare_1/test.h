/**
 * @File     : test.h
 * @Author   : Abdullah Younis
 *
 * NOTE: This test assumes an x86 architecture.
 */

#ifndef LIBFIPC_TEST_PING_PONG_BARE_1
#define LIBFIPC_TEST_PING_PONG_BARE_1

#include "../libfipc_test.h"

#define TRANSACTIONS	1000000
#define REQUESTER_CPU	0
#define RESPONDER_CPU	2

// Thread Locks
static DECLARE_COMPLETION(requester_comp);
static DECLARE_COMPLETION(responder_comp);

volatile cache_line_t CACHE_ALIGNED line;

volatile uint64_t CACHE_ALIGNED resp_sequence = 1; 
volatile uint64_t CACHE_ALIGNED req_sequence  = 1;

#endif
