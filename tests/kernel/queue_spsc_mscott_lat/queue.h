/**
 * @File     : queue.h
 * @Author   : Abdullah Younis
 */

#ifndef LIBFIPC_TEST_QUEUE
#define LIBFIPC_TEST_QUEUE

#include "../libfipc_test.h"

// Error Values
#define SUCCESS              0
#define NO_MEMORY            1
#define EMPTY_COLLECTION     2

// Types
typedef uint64_t data_t;

typedef struct linked_node_t
{
	data_t a;
	data_t b;
	data_t c;
	data_t d;
	data_t e;
	data_t f;
	data_t g;
	struct linked_node_t* next;

} node_t;

typedef struct queue_t
{
	node_t* head;
	node_t* tail;

	node_t header;

} queue_t;

int init_queue ( queue_t* q );
int free_queue ( queue_t* q );
int enqueue    ( queue_t* q, node_t* n );
int dequeue    ( queue_t* q, data_t* a, data_t* b, data_t* c, data_t* d, data_t* e, data_t* f, data_t* g );

#endif
