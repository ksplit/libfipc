/**
 * @File     : queue.h
 * @Author   : Abdullah Younis
 * @Author   : Jiwon Jeon
 */

#ifndef LIBFIPC_TEST_QUEUE
#define LIBFIPC_TEST_QUEUE

#ifndef LIBFIPC_TEST
#include "../libfipc_test.h"
#endif

#include "mcs.h"

#define CHANNEL_ORDER ilog2(sizeof(message_t)) + 13

// Error Values
#define SUCCESS              0
#define NO_MEMORY            1
#define EMPTY_COLLECTION     2

// Types
typedef uint64_t data_t;

//typedef node_t request_t;

// Types
typedef struct linked_node_t
{
	uint64_t data;
	struct linked_node_t* next;

} node_t;
 
typedef struct queue_t
{
	node_t* CACHE_ALIGNED head;
	node_t* CACHE_ALIGNED tail;

	mcslock* mcs_one_lock;

} queue_t;

int init_queue ( queue_t* q );
int free_queue ( queue_t* q );
int enqueue    ( queue_t* q, node_t* n );
int dequeue    ( queue_t* q, uint64_t* n );

#endif

