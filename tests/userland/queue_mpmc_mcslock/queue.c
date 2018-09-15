/**
 * @File     : queue.c
 * @Author   : Abdullah Younis
 * @Author   : Jeonghoon Lee
 */


#include "queue.h"

int init_queue ( queue_t* q )
{
	q->head = NULL;
	q->tail = NULL;

	mcs_init_global( &(q->mcs_one_lock) );

	return SUCCESS;
}

int free_queue ( queue_t* q )
{
	return SUCCESS;
}

// Enqueue
int enqueue ( queue_t* q, node_t* r )
{
	qnode I;
	r->next = NULL;
	
	mcs_lock( &(q->mcs_one_lock), &I );

	if ( q->tail )
	{
		q->tail->next = r;
		q->tail	= r;
	}
	else
	{
		q->head = r;
		q->tail = r;
	}

	mcs_unlock( &(q->mcs_one_lock), &I );

	return SUCCESS;
}


// Dequeue

int dequeue ( queue_t* q, uint64_t* data )
{
	qnode I;

	mcs_lock( &(q->mcs_one_lock), &I );

	node_t* temp = q->head;

	if ( !temp )
	{
		mcs_unlock( &(q->mcs_one_lock), &I );
		return EMPTY_COLLECTION;
	}

	*data = temp->data;
	if ( q->head == q->tail )
	{
		q->tail = NULL;
	}
	q->head = temp->next;

	mcs_unlock( &(q->mcs_one_lock), &I );

	return SUCCESS;
}

