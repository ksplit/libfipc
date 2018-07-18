/**
 * @File     : queue.c
 * @Author   : Abdullah Younis
 * @Author   : Jeonghoon Lee
 */

#include "queue.h"

void mcs_init ( mcslock *L )
{
    L->v = NULL;
    for (int i = 0; i < MAX_MCS_LOCKS; i++)
        if ( !lock_used[i].v )
        {
            if ( cmp_and_swap_atomic(&lock_used[i], 0, 1) )
            {
                L->lock_idx = i;
                return;
            }
        }
    die("mcs_init: Oops");
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
            nop_pause();
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
            nop_pause();
        }
    }
    ((struct qnode *)mynode->next)->locked = 0;
}


int init_queue ( queue_t* q )
{

}

int free_queue ( queue_t* q )
{

}

// Enqueue

int enqueue ( queue_t* q, request_t* r )
{
	r->next = NULL;

	// Acquire Lock, Enter Critical Section
	mcs_lock( &q->lock );

	q->tail->next = r;
	q->tail       = r;
	
	// Release Lock, Exit Critical Section
	mcs_unlock( &q->lock );
	return SUCCESS;
}

// Dequeue

int dequeue ( queue_t* q, uint64_t* data )
{
	request_t* temp;
	request_t* new_head;

	// Acquire Lock, Enter Critical Section
	mcs_lock( &q->lock );

	temp     = q->head;
	new_head = q->head->next;

	if ( new_head == NULL )
	{
		mcs_unlock( &q->lock );
		return EMPTY_COLLECTION;
	}

	*data   = new_head->data;
	q->head = new_head;

	// Release Lock, Exit Critical Section
	mcs_unlock( &q->lock );
	
	return SUCCESS;
}

