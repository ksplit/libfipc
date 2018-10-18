/**
 * @File     : queue.h
 * @Author   : Abdullah Younis
 */

#ifndef LIBFIPC_TEST_QUEUE
#define LIBFIPC_TEST_QUEUE

#include "../libfipc_test.h"

#define CHANNEL_ORDER ilog2(sizeof(message_t)) + 16

// Error Values
#define SUCCESS              0
#define NO_MEMORY            1
#define EMPTY_COLLECTION     2

// Types
struct node_t;

typedef struct pointer_t
{
	struct node_t* ptr;
	uint64_t count;
} pointer_t;

typedef struct node_t 
{
	uint64_t data;
	uint64_t prod_id;
	uint64_t cons_id;
	pointer_t next;	
} node_t;

typedef struct queue_t
{
	node_t header;
	pointer_t head;
	pointer_t tail;

} queue_t;


int init_queue(queue_t* q);
int free_queue(queue_t* q);
int enqueue(queue_t* q, node_t* r);
int dequeue(queue_t* q, uint64_t* data);

#endif	
