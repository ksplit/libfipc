/**
 * @File     : queue.c
 * @Author   : Abdullah Younis
 */

#include "queue.h"

#define pr_err printf
// Constructor

int init_queue ( queue_t* q )
{
	fipc_test_create_channel( CHANNEL_ORDER, &q->head, &q->tail );

	if ( q->head == NULL || q->tail == NULL )
	{
		pr_err( "%s\n", "Error while creating channel" );
		return -1;
	}
        thread_spin_init(&(q->H_lock));
	thread_spin_init(&(q->T_lock));
	return SUCCESS;
}

// Destructor

int free_queue ( queue_t* q )
{
	fipc_test_free_channel( CHANNEL_ORDER, q->head, q->tail );
	return SUCCESS;
}


// Enqueue

int enqueue_blk ( queue_t* q, node_t* node )
{
	message_t* msg;

	fipc_test_blocking_send_start(q->head, &msg );
	msg->regs[0] = (uint64_t)node;
	fipc_send_msg_end ( q->head, msg );

	return SUCCESS;
}

// Dequeue

int dequeue_blk ( queue_t* q, node_t** n )
{
	message_t* msg;

	fipc_test_blocking_recv_start(q->tail, &msg);
	*n = (node_t*)msg->regs[0];
	fipc_recv_msg_end( q->tail, msg );
	
	return SUCCESS;
}

int enqueue ( queue_t* q, node_t* node )
{
	message_t* msg;
	thread_spin_lock( &(q->T_lock) );

	if (fipc_send_msg_start( q->head, &msg ) != 0){
		thread_spin_unlock( &(q->T_lock) );
		return NO_MEMORY;
	}

	msg->regs[0] = (uint64_t)node;
	fipc_send_msg_end ( q->head, msg );
	thread_spin_unlock( &(q->T_lock) );
	
	return SUCCESS;
}

// Dequeue

int dequeue ( queue_t* q, node_t** node )
{
	message_t* msg;

	if (fipc_recv_msg_start( q->tail, &msg) != 0){
		return EMPTY_COLLECTION;
	}

	*node = (node_t*)msg->regs[0];
	fipc_recv_msg_end( q->tail, msg );

	return SUCCESS;
}

