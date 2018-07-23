/**
 * @File     : bqueue.h
 * @Author   : Abdullah Younis
 *
 * CITE: https://github.com/olibre/B-Queue/blob/master/
 */

#ifndef LIBFIPC_TEST_QUEUE
#define LIBFIPC_TEST_QUEUE

#include "../libfipc_test.h"

#define QUEUE_SIZE (1024 * 8) 
#define BATCH_SIZE (QUEUE_SIZE/16)
#define CONS_BATCH_SIZE BATCH_SIZE
#define PROD_BATCH_SIZE BATCH_SIZE
#define BATCH_INCREMENT (BATCH_SIZE/2)

#define CONGESTION_PENALTY (1000) /* cycles */

// Error Values
#define SUCCESS              0
#define NO_MEMORY            1
#define EMPTY_COLLECTION     2
#define BUFFER_FULL         -1
#define BUFFER_EMPTY        -2

// Types
typedef uint64_t data_t;

typedef struct node {
	uint64_t field;	
} node_t;


typedef struct CACHE_ALIGNED queue_t
{
	/* Mostly accessed by producer. */
	volatile	uint64_t	head;
	volatile	uint64_t	batch_head;

	/* Mostly accessed by consumer. */
	volatile	uint64_t	tail CACHE_ALIGNED;
	volatile	uint64_t	batch_tail;
	unsigned long	batch_history;

	/* accessed by both producer and comsumer */
	data_t	data[QUEUE_SIZE] CACHE_ALIGNED;

} queue_t;

int init_queue ( queue_t* q );
int free_queue ( queue_t* q );
int enqueue    ( queue_t* q, data_t  d );
int dequeue    ( queue_t* q, data_t* d );

#endif
