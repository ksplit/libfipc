/**
 * @File     : queue.h
 * @Author   : Abdullah Younis
 */

#ifndef LIBFIPC_TEST_QUEUE
#define LIBFIPC_TEST_QUEUE

#include "../libfipc_test.h"
#include "mcslock.h"

#define CHANNEL_ORDER ilog2(sizeof(message_t)) + 16

// Error Values
#define SUCCESS              0
#define NO_MEMORY            1
#define EMPTY_COLLECTION     2

// Types
typedef uint64_t data_t;

typedef struct linked_node_t 
{

	uint64_t data;
	uint64_t produ_id;
	uint64_t consu_id;
	struct linked_node_t* next;

} node_t;


typedef struct queue_t
{
	node_t* head;
	node_t* tail;

	node_t header;

	mcslock* lock;
} queue_t;

int init_queue(queue_t* q);
int free_queue(queue_t* q);
int enqueue(queue_t* q, node_t* r);
int dequeue(queue_t* q, uint64_t* data);


#endif
