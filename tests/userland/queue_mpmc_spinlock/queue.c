/**
 * @File     : queue.c
 * @Author   : Abdullah Younis
 * @Author   : Jeonghoon Lee
 */


#include "queue.h"

int init_queue ( queue_t* q )
{
	q->header.next = NULL;

//	q->head = &(q->header);
//	q->tail = &(q->header);
	q->head = NULL;
	q->tail = NULL;


//	mcs_init_global( &(q->H_lock) );
	thread_spin_init( &(q->T_lock) );

	return SUCCESS;
}

int free_queue ( queue_t* q )
{
//	fipc_test_free_channel( CHANNEL_ORDER, q->head, q->tail );
	return SUCCESS;
}

// Enqueue

int enqueue ( queue_t* q, node_t* r )
{
	thread_spin_lock( &(q->T_lock) );
	
	r->next = NULL;

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

	thread_spin_unlock( &(q->T_lock) );

	return SUCCESS;
}


// Dequeue

int dequeue ( queue_t* q, uint64_t* data )
{
	thread_spin_lock( &(q->T_lock) );

	node_t* temp = q->head;

	if ( !temp )
	{
		thread_spin_unlock( &(q->T_lock) );
		return EMPTY_COLLECTION;
	}

	*data = temp->data;
	if ( q->head == q->tail )
	{
		q->tail = NULL;
	}
	q->head = temp->next;

	thread_spin_unlock( &(q->T_lock) );

	return SUCCESS;
}

