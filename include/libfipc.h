/**
 * @File     : libfipc.h
 * @Author   : Anton Burtsev
 * @Author   : Scotty Bauer
 * @Author   : Charles Jacobsen
 * @Author   : Abdullah Younis
 * @Copyright: University of Utah
 *
 * Asynchronous fast-IPC library using shared memory.
 */

#ifndef LIBFIPC_H
#define LIBFIPC_H

#include <libfipc_types.h>
#include <libfipc_platform.h>

/**
 * fipc_buffer_init -- Initialize the shared memory buffer for fipc
 *
 * @buf_order: The buffer is 2^buf_order bytes
 * @buffer   : Pointer to shared memory buffer
 *
 * NOTE: This must be called exactly once per channel before any IPC use.
 *
 * NOTE: @buffer *must* be exactly 2^buf_order bytes, otherwise there
 *       will be memory corruption.
 */

int fipc_buffer_init ( uint32_t buf_order, void* buffer );

/**
 * fipc_channel_init -- Initialize headers; link headers with buffer
 *
 * @h1       : One of the channel headers
 * @h2       : The other channel header
 * @buf_order: The buffer is 2^buf_order bytes
 * @buffer   : Pointer to shared memory buffer
 *
 * This function fully sets up a two-way IPC channel and should only be
 * called once per channel and after fipc_buffer_init. This cannot be
 * called alongside fipc_header_init.
 *
 * NOTE:   fipc_header_init(h1, 0, ...)
 *       + fipc_header_init(h2, 1, ...)
 *       -------------------------
 *       = fipc_channel_init(h1, h2, ..)
 */

int fipc_channel_init ( header_t* h1, header_t* h2, uint32_t buf_order, void* buffer );

/**
 * fipc_header_init -- Initialize header and link with buffer
 *
 * @head     : The channel header
 * @client   : Determines the side of the buffer seen as tx (0 or 1)
 * @buf_order: The buffer is 2^buf_order bytes
 * @buffer   : Pointer to shared memory buffer
 *
 * This function links one header to one side of the buffer. For a fully
 * functioning duplex communication a partner header needs to be
 * initialized and linked to the same buffer on the other side.
 * This cannot be called alongside fipc_channel_init.
 *
 * NOTE: In most scenarios this function will not need to be called,
 *       rather, fipc_channel_init should be favored.
 */

int fipc_header_init ( header_t* head, int client, uint32_t buf_order, void* buffer );

/**
 * fipc_send_msg_start -- Allocate a slot from tx buffer for sending
 *
 * @head: The channel header
 * @msg : Out param, the allocated slot
 *
 * If there are no free slots, returns -EWOULDBLOCK.
 *
 * IMPORTANT: If the sender fails to invoke fipc_send_msg_end, this
 *            could introduce some delay and re-ordering of messages.
 *            The slot will not be marked as ready to receive, for the
 *            receiver to pick up. So, make sure the code in between
 *            start and end cannot fail.
 *
 * IMPORTANT: This function is NOT thread safe.
 */

int fipc_send_msg_start ( header_t* head, message_t** msg );

/**
 * fipc_send_msg_end -- Mark a message as ready and sent
 *
 * @head: The channel header
 * @msg : The message we are sending
 *
 * NOTE: This function is thread safe.
 */

int fipc_send_msg_end ( header_t* head, message_t* msg );

/**
 * fipc_recv_msg_start -- Receive the next message if available
 *
 * @head: The channel header
 * @msg : Out param, the received message
 *
 * If there are no messages to be received, returns -EWOULDBLOCK.
 *
 * IMPORTANT: If the caller fails to invoke fipc_recv_msg_end, the
 *            sender will potentially block waiting for the slot to
 *            become free. So, make sure your code cannot fail between
 *            start/end.
 *
 * IMPORTANT: This function is NOT thread safe.
 */

int fipc_recv_msg_start ( header_t* head, message_t** msg );

/**
 * fipc_recv_msg_if -- fipc_recv_msg_start with an added condition
 *
 * @head: The channel header
 * @pred: The condition under which we should receive a message
 * @data: Context data to pass to @pred
 * @msg : Out param, the received message
 *
 * This is like fipc_recv_msg_start, but if there is a message to be
 * received, libfipc will allow @pred to peek at the message to see if
 * the caller wants to receive it (by looking at values in the message).
 * libfipc will pass along @data to @pred, providing context.
 *
 * @pred should return non-zero to indicate the caller should receive the
 * message, and zero if no.
 *
 * IMPORTANT: This function is NOT thread safe.
 */

int fipc_recv_msg_if ( header_t* head, int (*pred)(message_t*, void*), void* data, message_t** msg );

/**
 * fipc_recv_msg_end -- Mark a message as received ready to be re-used

 * @head: The channel header
 * @msg : the message to mark as received
 *
 * NOTE: This function is thread safe.
 */

int fipc_recv_msg_end ( header_t* head, message_t* msg );

// =============================================================
// ------------------ MESSAGE ACCESSORS ------------------------
// =============================================================
// The use of these functions makes your code independent of the
// structure of our code, however, they are not required.

#define FIPC_MK_REG_ACCESS(idx)                         \
static inline                                           \
uint64_t fipc_get_reg##idx ( message_t *msg )           \
{                                                       \
	FIPC_BUILD_BUG_ON(idx >= FIPC_NR_REGS);             \
	return msg->regs[idx];                              \
}                                                       \
static inline                                           \
void fipc_set_reg##idx ( message_t *msg, uint64_t val ) \
{                                                       \
	msg->regs[idx] = val;                               \
}

FIPC_MK_REG_ACCESS(0)
FIPC_MK_REG_ACCESS(1)
FIPC_MK_REG_ACCESS(2)
FIPC_MK_REG_ACCESS(3)
FIPC_MK_REG_ACCESS(4)
FIPC_MK_REG_ACCESS(5)
FIPC_MK_REG_ACCESS(6)

static inline
uint32_t fipc_get_flags ( message_t *msg )
{
	return msg->flags;
}

static inline
void fipc_set_flags ( message_t *msg, uint32_t flags )
{
	msg->flags = flags;
}

#endif
