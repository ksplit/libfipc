/**
 * @File     : test.h
 * @Author   : Abdullah Younis
 *
 * This test sends establishes a requester thread and a responder thread on
 * seperate processers, and records certain cpu/cache events.
 *
 * The events can be programmed in main.c in the FILL_EVENT_OS macros
 * Event ids and mask ids can be found in your cpu's architecture manual
 * Emulab's d710 (table 19-17) and d820 (table 19-13, 19-15) machines can use this link:
 * https://www.intel.com/content/www/us/en/architecture-and-technology/64-ia-32-architectures-software-developer-vol-3b-part-2-manual.html
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
