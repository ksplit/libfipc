/**
 * @File     : test.h
 * @Author   : Abdullah Younis
 *
 * This test sends a cache line to another processor.
 *
 * NOTE: This test assumes an x86 architecture.
 */

 #include "../libfipc_test.h"

#define TRANSACTIONS	10000000
#define REQUESTER_CPU	0
#define RESPONDER_CPU	2

// Thread Locks
pthread_mutex_t requester_mutex;
pthread_mutex_t responder_mutex;

volatile cache_aligned_ull_int_t req_line;
volatile cache_aligned_ull_int_t resp_line; // possible more on this cacheline

cache_aligned_ull_int_t resp_sequence = 1; 
cache_aligned_ull_int_t req_sequence = 1;
