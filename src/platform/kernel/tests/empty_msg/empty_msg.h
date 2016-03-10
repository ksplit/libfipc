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

/* thread main functions */
int callee(void *_callee_channel_header);
int caller(void *_caller_channel_header);

#endif /* LIBFIPC_EMPTY_MSG_TEST_H */
