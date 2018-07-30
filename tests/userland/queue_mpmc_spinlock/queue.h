/**
 * @File     : queue.h
 * @Author   : Abdullah Younis
 */

#ifndef LIBFIPC_TEST_QUEUE
#define LIBFIPC_TEST_QUEUE

#include "../libfipc_test.h"
#include "spinlock.h"

#define CHANNEL_ORDER ilog2(sizeof(message_t)) + 16

// Error Values
#define SUCCESS              0
#define NO_MEMORY            1
#define EMPTY_COLLECTION     2

// Types
typedef uint64_t data_t;

typedef struct node {
	uint64_t CACHE_ALIGNED field;	
	struct node* next;
} node_t;

typedef struct queue_t
{
	header_t* CACHE_ALIGNED head;
	header_t* CACHE_ALIGNED tail;

	struct thread_spinlock H_lock;	
	struct thread_spinlock T_lock;

} queue_t;

int init_queue(queue_t* q);
int free_queue(queue_t* q);
int enqueue(queue_t* q, node_t* node);
int dequeue(queue_t* q, node_t** node);
int enqueue_blk(queue_t* q, node_t* node);
int dequeue_blk(queue_t* q, node_t** node);


#endif
