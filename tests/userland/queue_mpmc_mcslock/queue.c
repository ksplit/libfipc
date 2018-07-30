/**
 * @File     : queue.c
 * @Author   : Abdullah Younis
 * @Author   : Jeonghoon Lee
 */


#include "queue.h"

int init_queue ( queue_t* q )
{
	fipc_test_create_channel( CHANNEL_ORDER, &q->head, &q->tail );

	if ( q->head == NULL || q->tail == NULL )
	{
		pr_err( "%s\n", "Error while creating channel" );
		return -1;
	}
	mcs_init( &(q->H_lock) );
	mcs_init( &(q->T_lock) );

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
	mcs_lock( &(q->T_lock) );

	if (fipc_send_msg_start( q->head, &msg ) != 0)
	{
		mcs_unlock( &(q->T_lock) );
		return NO_MEMORY;
	}

	msg->regs[0] = (uint64_t)node;
	fipc_send_msg_end ( q->head, msg );
	mcs_unlock( &(q->T_lock) );

	return SUCCESS;
}


// Dequeue

int dequeue ( queue_t* q, node_t** node )
{

	message_t* msg;
	mcs_lock( &(q->H_lock) );

	if (fipc_recv_msg_start( q->tail, &msg) != 0)
	{
		mcs_unlock( &(q->H_lock) );
		return EMPTY_COLLECTION;
	}

	*node = (node_t*)msg->regs[0];
	fipc_recv_msg_end( q->tail, msg );
	mcs_unlock( &(q->H_lock) );

	return SUCCESS;
}
