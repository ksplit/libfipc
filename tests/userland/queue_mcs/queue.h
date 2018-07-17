/**
 * @File     : queue.h
 * @Author   : Abdullah Younis
 */

#ifndef LIBFIPC_TEST_QUEUE
#define LIBFIPC_TEST_QUEUE

#include <asm-generic/qspinlock.h>

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

	node_t header;

	struct qspinlock H_lock;
	struct qspinlock T_lock;
    struct qnode lock;

} queue_t;


struct qnode {
    volatile void *next;
    volatile char locked;
    char __pad[0] __attribute__((aligned(CACHELINE)));
};

typedef struct {
    struct qnode *v __align__;
    int lock_idx __align__;
} mcslock;

int init_queue ( queue_t* q );
int free_queue ( queue_t* q );
int enqueue    ( queue_t* q, request_t* r );
int dequeue    ( queue_t* q, uint64_t* data );

#endif
