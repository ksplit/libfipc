/**
 * @File     : queue.h
 * @Author   : Abdullah Younis
 */

#ifndef LIBFIPC_TEST_QUEUE
#define LIBFIPC_TEST_QUEUE

#include <linux/vmalloc.h>
#include <linux/spinlock.h>

#define PREALLOCATED_NODES 256

// Error Values
#define SUCCESS              0
#define NO_MEMORY            1
#define EMPTY_COLLECTION     2

// Types

typedef struct linked_node_t
{
	uint64_t data;
	struct linked_node_t* next;

} node_t;

typedef node_t request_t;

typedef struct queue_t
{
	node_t* head;
	node_t* tail;

	node_t*  node_table;
	uint64_t logical_size;
	uint64_t physical_size;

	spinlock_t queue_lock;
	spinlock_t node_table_lock;

} queue_t;

int init_queue    ( queue_t* q );
int free_queue    ( queue_t* q );
int enqueue       ( queue_t* q, request_t* r );
int dequeue       ( queue_t* q, request_t* r );
int alloc_request ( queue_t* q, request_t* r );

#endif
