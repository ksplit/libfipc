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

// Thread Locks
pthread_mutex_t requester_mutex;
pthread_mutex_t responder_mutex;

/* THC MESSAGES ------------------------------------------------------------ */

#define THC_RESERVED_MSG_FLAG_BITS (2 + AWE_TABLE_ORDER)

/* Message type is in low 2 bits of flags */
static inline uint32_t thc_get_msg_type(struct fipc_message *msg)
{
#if defined(SIMPLE_MSG_TYPES)
	return msg->regs[0];
#else
	return fipc_get_flags(msg) & 0x3;
#endif
}
static inline void thc_set_msg_type(struct fipc_message *msg, uint32_t type)
{
#if defined(SIMPLE_MSG_TYPES)
	msg->regs[0] = type; 
#else
	fipc_set_flags(msg,
		/* erase old type, and mask off low 2 bits of type */
		(fipc_get_flags(msg) & ~0x3) | (type & 0x3));
#endif
}
/* Message id is in bits 2..(2 + AWE_TABLE_ORDER) bits */
static inline uint32_t thc_get_msg_id(struct fipc_message *msg)
{
#if defined(SIMPLE_MSG_TYPES)
	return msg->regs[1];
#else
	/* shift off type bits, and mask off msg id */
	return (fipc_get_flags(msg) >> 2) & ((1 << AWE_TABLE_ORDER) - 1);
#endif
}
static inline void thc_set_msg_id(struct fipc_message *msg,
				uint32_t msg_id)
{
#if defined(SIMPLE_MSG_TYPES)
	msg->regs[1] = msg_id; 
#else
	uint32_t flags = fipc_get_flags(msg);
	/* erase old msg id, if any */
	flags &= ~(((1 << AWE_TABLE_ORDER) - 1) << 2);
	/* mask off relevant bits of msg id (to ensure it is in range),
	 * and install in flags. */
	flags |= (msg_id & ((1 << AWE_TABLE_ORDER) - 1)) << 2;
	fipc_set_flags(msg, flags);
#endif
}

static inline uint32_t thc_get_request_cookie(struct fipc_message *request)
{
    return thc_get_msg_id(request);
}

static inline void thc_kill_request_cookie(uint32_t request_cookie)
{
    awe_mapper_remove_id(request_cookie);
}

#endif
