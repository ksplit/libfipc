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

#define TRANSACTIONS	100000
#define REQUESTER_CPU	0
#define RESPONDER_CPU	2

// Thread Locks
static DECLARE_COMPLETION(requester_comp);
static DECLARE_COMPLETION(responder_comp);

volatile cache_aligned_ull_int_t req_line;
volatile cache_aligned_ull_int_t resp_line; // possible more on this cacheline

cache_aligned_ull_int_t resp_sequence = 1; 
cache_aligned_ull_int_t req_sequence = 1;

#endif
