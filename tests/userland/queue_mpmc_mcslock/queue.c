/**
 * @File     : queue.c
 * @Author   : Abdullah Younis
 * @Author   : Jeonghoon Lee
 */


#include "queue.h"

int init_queue ( queue_t* q )
{
	q->header.next = NULL;

	q->head = &(q->header);
	q->tail = &(q->header);

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
	qnode *I = malloc( sizeof(qnode) );
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
/*
	message_t* msg;
	qnode *I = malloc(sizeof(qnode));
	mcs_init_local(I);
	mcs_lock( &(q->T_lock),I );

	if (fipc_send_msg_start( q->head, &msg ) != 0)
	{
		mcs_unlock( &(q->T_lock),I );
		return NO_MEMORY;
	}

	msg->regs[0] = (uint64_t)node;
	fipc_send_msg_end ( q->head, msg );
	mcs_unlock( &(q->T_lock),I );
*/
	return SUCCESS;
}


// Dequeue

int dequeue ( queue_t* q, uint64_t* data )
{
	qnode *I = malloc( sizeof(qnode) );
	mcs_init_local( I );

	mcs_lock( &(q->H_lock), I );

//	node_t* temp = q->head;
	node_t* new_head = q->head->next;

	if ( !new_head )
	{
		mcs_unlock( &(q->H_lock), I );
		return EMPTY_COLLECTION;
	}

	*data   = new_head->data;
	q->head = new_head;

	mcs_unlock( &(q->H_lock), I );
/*
	message_t* msg;
	qnode *I = malloc(sizeof(qnode));
	mcs_init_local(I);	
	mcs_lock( &(q->H_lock),I );

	if (fipc_recv_msg_start( q->tail, &msg) != 0)
	{
		mcs_unlock( &(q->H_lock),I );
		return EMPTY_COLLECTION;
	}

	*node = (node_t*)msg->regs[0];
	fipc_recv_msg_end( q->tail, msg );
	mcs_unlock( &(q->H_lock),I );
*/
	return SUCCESS;
}
