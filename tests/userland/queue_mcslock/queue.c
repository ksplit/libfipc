/**
 * @File     : queue.c
 * @Author   : Abdullah Younis
 * @Author   : Jeonghoon Lee
 */

#include "queue.h"

#ifndef __KERNEL__
#define pr_err printf
#endif

static inline uint64_t
cmp_and_swap_atomic(mcslock* L, uint64_t cmpval, uint64_t newval)
{
    uint64_t out;
    __asm__ volatile(
                "lock; cmpxchgq %2, %1"
                : "=a" (out), "+m" ((L->v)->locked)
                : "q" (newval), "0"(cmpval)
                : "cc");
    return out == cmpval;
}

static inline uint64_t
fetch_and_store(mcslock *L, uint64_t val) 
{
    __asm__ volatile(
                "lock; xchgq %0, %1\n\t"
                : "+m" (L->v), "+r" (val)
                : 
                : "memory", "cc");
    return val;
}

static inline uint64_t
cmp_and_swap(mcslock *L, uint64_t cmpval, uint64_t newval)
{
    uint64_t out;
    __asm__ volatile(
                "lock; cmpxchgq %2, %1"
                : "=a" (out), "+m" (L->v)
                : "q" (newval), "0"(cmpval)
                : "cc");
    return out == cmpval;
}

void mcs_init ( mcslock *L )
{
    L->v = NULL;
    for (int i = 0; i < MAX_MCS_LOCKS; i++)
        if ( !lock_used[i].v )
        {
            if (cmp_and_swap_atomic( &lock_used[i], 0, 1) )
            {
                L->lock_idx = i;
                return;
            }
        }
    pr_err("mcs init: Oops");
}

void mcs_lock ( mcslock *L )
{
    volatile struct qnode *mynode = &I[L->lock_idx];
    mynode->next = NULL;
    struct qnode *predecessor = (struct qnode *)fetch_and_store(L, (uint64_t)mynode);
    if (predecessor) {
        mynode->locked = 1;
        predecessor->next = mynode;
        while (mynode->locked)
        {
            //nop_pause();
        }
    }
}

void mcs_unlock ( mcslock *L )
{
    volatile struct qnode *mynode = &I[L->lock_idx];
    if (!mynode->next) {
        if ( cmp_and_swap(L, (uint64_t)mynode, 0) ) {
            return;
        }
        while ( !mynode->next )
        {
            //nop_pause();
        }
    }
    ((struct qnode *)mynode->next)->locked = 0;
}


int init_queue ( queue_t* q )
{
	fipc_test_create_channel( CHANNEL_ORDER, &q->head, &q->tail );

	if ( q->head == NULL || q->tail == NULL )
	{
		pr_err( "%s\n", "Error while creating channel" );
		return -1;
	}

	return SUCCESS;

}

int free_queue ( queue_t* q )
{
	fipc_test_free_channel( CHANNEL_ORDER, q->head, q->tail );
	return SUCCESS;
}

// Enqueue

int enqueue ( queue_t* q, node_t* node )
{
	message_t* msg;

	if (fipc_send_msg_start( q->head, &msg ) != 0)
		return NO_MEMORY;

	msg->regs[0] = (uint64_t)node;
	fipc_send_msg_end ( q->head, msg );

	return SUCCESS;
}


// Dequeue

int dequeue ( queue_t* q, node_t** node )
{
	message_t* msg;

	if (fipc_recv_msg_start( q->tail, &msg) != 0)
		return EMPTY_COLLECTION;

	*node = (node_t*)msg->regs[0];
	fipc_recv_msg_end( q->tail, msg );

	return SUCCESS;
}
