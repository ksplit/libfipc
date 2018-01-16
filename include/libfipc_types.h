/**
 * @File     : libfipc_types.h
 * @Author   : Anton Burtsev
 * @Author   : Scotty Bauer
 * @Author   : Charles Jacobsen
 * @Author   : Abdullah Younis
 * @Copyright: University of Utah
 *
 * This library contains data structure definitions for the fipc system.
 */

#ifndef LIBFIPC_TYPES_H
#define LIBFIPC_TYPES_H

#include <libfipc_platform_types.h>

// Assumed cache line size, in bytes
#ifndef FIPC_CACHE_LINE_SIZE
	#define FIPC_CACHE_LINE_SIZE 64
#endif

// Type modifier that aligns the variable to a cache line
#ifndef CACHE_ALIGNED
	#define CACHE_ALIGNED __attribute__((aligned(FIPC_CACHE_LINE_SIZE)))
#endif

// Type modifier that aligns the variable to a cache line pair
#ifndef PAIR_ALIGNED
	#define PAIR_ALIGNED __attribute__((aligned(2*FIPC_CACHE_LINE_SIZE)))
#endif

// The number of 64 bit registers in a single message.
#define FIPC_NR_REGS ((FIPC_CACHE_LINE_SIZE-8)/8)

// The amount of padding needed for the header
#define FIPC_HEADER_PADDING ((FIPC_CACHE_LINE_SIZE-40)/8)

/**
 * A single message is the smallest unit of the fipc system and fully
 * occupies a cache line. This is one the cache traffic optimizations
 * made by the library.
 */
typedef struct CACHE_ALIGNED fipc_message
{
	volatile uint32_t msg_status; // The status of the message
	uint32_t flags;               // Not touched by libfipc
	uint64_t regs[FIPC_NR_REGS];  // Not touched by libfipc

} message_t;

/**
 * A message pair couples the client and server cache lines, which
 * efficiently utilizes prefetching.
 */
typedef struct CACHE_ALIGNED fipc_message_pair
{
	message_t line[2]; // Client's and Server's line

} pair_t;

/**
 * A channel header which contains data necessary to send and receive
 * message across the IPC mechanism. This is provided there is a linked
 * analogue to communicate with.
 */
typedef struct CACHE_ALIGNED chan_header
{
	uint64_t tx_idx;  // Index on transmit side of buffer
	uint64_t rx_idx;  // Index on receive side of buffer
	uint64_t tx_side; // Determines the buffer side seen as tx (0 or 1)
	uint64_t mask;    // Mask for easy modular index calculation
	pair_t*  buffer;  // Pointer to the IPC circular, pair-wise buffer

	// Extra padding
	uint64_t padding[FIPC_HEADER_PADDING];

} header_t;

#endif
