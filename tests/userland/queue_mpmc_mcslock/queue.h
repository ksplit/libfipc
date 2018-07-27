/**
 * @File     : queue.h
 * @Author   : Abdullah Younis
 */

#ifndef LIBFIPC_TEST_QUEUE
#define LIBFIPC_TEST_QUEUE

//#include "../libfipc_test.h"
#include "../libfipc_test.h"

#define CHANNEL_ORDER ilog2(sizeof(message_t)) + 13

// Error Values
#define SUCCESS              0
#define NO_MEMORY            1
#define EMPTY_COLLECTION     2

#define MAX_MCS_LOCKS        2

// Types
typedef uint64_t data_t;

struct qnode {
    volatile void* CACHE_ALIGNED next; 
    volatile char CACHE_ALIGNED locked; 
}; 

typedef struct {
    struct qnode* CACHE_ALIGNED v;
    int CACHE_ALIGNED lock_idx;
} mcslock;

typedef struct node {
	uint64_t CACHE_ALIGNED field;	
} node_t;

typedef struct queue_t
{
	header_t* CACHE_ALIGNED head;
	header_t* CACHE_ALIGNED tail;

	mcslock H_lock;
	mcslock T_lock;

} queue_t;

volatile struct qnode I[MAX_MCS_LOCKS];
mcslock lock_used[MAX_MCS_LOCKS];

int init_queue ( queue_t* q );
int free_queue ( queue_t* q );
int enqueue    ( queue_t* q, node_t* n );
int dequeue    ( queue_t* q, node_t** n );

#endif

