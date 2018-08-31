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


	mcs_init_global( &(q->H_lock) );
	mcs_init_global( &(q->T_lock) );

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
	qnode *I = (qnode *)malloc( sizeof(qnode) );
	mcs_init_local( I );
	
	r->next = NULL;
	mcs_lock( &(q->T_lock), I );

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

	mcs_unlock( &(q->T_lock), I );
	free( I );

	return SUCCESS;
}


// Dequeue

int dequeue ( queue_t* q, uint64_t* data )
{
	qnode *I = (qnode *)malloc( sizeof(qnode) );
	mcs_init_local( I );

	mcs_lock( &(q->T_lock), I );

	node_t* temp = q->head;

	if ( !temp )
	{
		mcs_unlock( &(q->T_lock), I );
		free( I );
		return EMPTY_COLLECTION;
	}

	node_t* new_head = q->head->next;

	*data = temp->data;
	if ( q->head == q->tail )
	{
		q->tail = NULL;
	}
	q->head = temp->next;

	mcs_unlock( &(q->T_lock), I );
	free( I );

	return SUCCESS;
}

