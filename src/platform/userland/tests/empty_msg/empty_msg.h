/**
 * @File     : empty_msg.h
 * @Author   : Charles Jacobsen
 * @Author   : Abdullah Younis
 * @Copyright: University of Utah
 *
 * This test sends an empty message in a round trip using the fipc library.
 *
 * NOTE: This program assumes an x86 architecture.
 */

#ifndef LIBFIPC_EMPTY_MSG_TEST_H
#define LIBFIPC_EMPTY_MSG_TEST_H

#include "../libfipc_test.h"

#define TRANSACTIONS 10000

// Thread Main Functions
void* callee(void *_callee_channel_header);
void* caller(void *_caller_channel_header);

// Thread Locks
pthread_mutex_t callee_mutex;
pthread_mutex_t caller_mutex;

#endif
