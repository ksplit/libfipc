/**
 * @File     : test.h
 * @Author   : Abdullah Younis
 *
 * NOTE: This test assumes an x86 architecture.
 */

#ifndef LIBFIPC_TEST_PING_PONG
#define LIBFIPC_TEST_PING_PONG

#include "../libfipc_test.h"

#define CHANNEL_ORDER ilog2(sizeof(message_t)) + 7

// Test Variables
static uint32_t transactions   = 300;
static uint8_t  controller_cpu = 0;
static uint8_t  slave_count    = 3;

module_param( transactions,     uint, 0 );
module_param( controller_cpu,   byte, 0 );
module_param( slave_count,      byte, 0 );

// Request Types
#define ENQUEUE 0
#define DEQUEUE 1
#define SIZE    2
#define CLEAR   3
#define HALT    4

// Error Values
#define SUCCESS              0
#define NO_MEMORY            1
#define INVALID_REQUEST_TYPE 2
#define EMPTY_COLLECTION     3

// Linked List Variables
typedef struct linked_node_t
{
	uint64_t data;
	struct linked_node_t* next;

} node_t;

// Thread Locks
struct completion controller_comp;

#endif
