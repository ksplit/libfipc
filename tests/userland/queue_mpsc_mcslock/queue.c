/**
 * @File     : queue.c
 * @Author   : Abdullah Younis
 * @Author   : Jeonghoon Lee
 */


#include "queue.h"

int init_queue ( queue_t* q )
{
	q->first.next = NULL;

	q->head = q->tail = &q->first;

	mcs_init_global( &q->H_lock );
	mcs_init_global( &q->T_lock );

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
	
	mcs_lock( &q->T_lock, &I );

	q->tail->next = r;
	q->tail = r;

	mcs_unlock( &q->T_lock, &I );

	return SUCCESS;
}


// Dequeue

int dequeue ( queue_t* q, uint64_t* data )
{
	qnode I;
	node_t* temp, * new_head;

	mcs_lock( &q->H_lock, &I );

	temp	 = q->head;
	new_head = temp->next;

	if ( new_head == NULL )
	{
		mcs_unlock( &q->H_lock, &I );
		return EMPTY_COLLECTION;
	}

	*data 	= new_head->data;
	q->head = new_head;

	mcs_unlock( &q->H_lock, &I );

	return SUCCESS;
}

