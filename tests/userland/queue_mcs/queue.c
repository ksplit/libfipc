/**
 * @File     : queue.c
 * @Author   : Abdullah Younis
 * @Author   : Jeonghoon Lee
 */

#include "queue.h"

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

