/*
 * empty_msg.h
 *
 * Internal defs
 *
 * Copyright: University of Utah
 */
#ifndef LIBFIPC_EMPTY_MSG_TEST_H
#define LIBFIPC_EMPTY_MSG_TEST_H

#include <libfipc.h>
#include "../test_Lib.h"

#define TRANSACTIONS 10000

// Thread Main Functions
void* callee(void *_callee_channel_header);
void* caller(void *_caller_channel_header);

// Thread Locks
pthread_mutex_t callee_mutex;
pthread_mutex_t caller_mutex;

#endif
