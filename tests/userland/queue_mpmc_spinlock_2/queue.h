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

#include "spinlock.h"

#define CHANNEL_ORDER ilog2(sizeof(message_t)) + 13

// Error Values
#define SUCCESS              0
#define NO_MEMORY            1
#define EMPTY_COLLECTION     2

// Types
typedef uint64_t data_t;

typedef struct node {
	uint64_t CACHE_ALIGNED field;
} node_t;

// node struct for queue
typedef struct qnode_t
{
	node_t* node;
	struct qnode_t* next;

} qnode_t;

typedef struct queue_t
{
	//header_t* CACHE_ALIGNED head;
	//header_t* CACHE_ALIGNED tail;
	qnode_t* CACHE_ALIGNED head;
	qnode_t* CACHE_ALIGNED tail;

	struct thread_spinlock H_lock;
	struct thread_spinlock T_lock;

} queue_t;

int init_queue ( queue_t* q );
int free_queue ( queue_t* q );
int enqueue    ( queue_t* q, node_t* node );
int dequeue    ( queue_t* q );

#endif

