/**
 * @File     : queue.h
 * @Author   : Abdullah Younis
 */

#ifndef LIBFIPC_TEST_QUEUE
#define LIBFIPC_TEST_QUEUE

#include <linux/spinlock.h>
#include "../libfipc_test.h"

#define CHANNEL_ORDER ilog2(sizeof(message_t)) + 12

// Error Values
#define SUCCESS              0
#define NO_MEMORY            1
#define EMPTY_COLLECTION     2

// Types
typedef message_t request_t;

typedef struct queue_t
{
	header_t* head;
	header_t* tail;

} queue_t;

int init_queue ( queue_t* q );
int free_queue ( queue_t* q );
int enqueue    ( queue_t* q, request_t* r );
int dequeue    ( queue_t* q, request_t** r );

#endif
