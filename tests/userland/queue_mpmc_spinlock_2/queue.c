/**
 * @File     : queue.c
 * @Author   : Abdullah Younis
 * @Author   : Jeonghoon Lee
 */


#include "queue.h"

int init_queue ( queue_t* q )
{
	q->head = q->tail = NULL;
/*
	fipc_test_create_channel( CHANNEL_ORDER, &q->head, &q->tail );

	if ( q->head == NULL || q->tail == NULL )
	{
		pr_err( "%s\n", "Error while creating channel" );
		return -1;
	}
*/
	thread_spin_init( &(q->H_lock) );
	thread_spin_init( &(q->T_lock) );

	return SUCCESS;
}

int free_queue ( queue_t* q )
{
//	fipc_test_free_channel( CHANNEL_ORDER, q->head, q->tail );
	return SUCCESS;
}

// Enqueue

int enqueue ( queue_t* q, node_t* node )
{
	thread_spin_lock( &(q->T_lock) );
	qnode_t* new = (qnode_t*) malloc(sizeof(qnode_t));
	new->node = node;
	new->next = NULL;

	if ( q->tail )
	{
		q->tail->next = new;
		q->tail->next = new;
	}
	else
	{
		q->head = new;
		q->tail = new;
	}

	thread_spin_unlock( &(q->T_lock) );
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

int dequeue ( queue_t* q )
{
	thread_spin_lock( &(q->H_lock) );

	qnode_t* temp = q->head;
	qnode_t* new_head = q->head->next;

	if ( !new_head )
	{
		free(q->head);
		q->head = NULL;
		
		thread_spin_unlock( &(q->H_lock) );
		return SUCCESS;
	}
	q->head = new_head;
	free(temp);

	thread_spin_unlock( &(q->H_lock) );
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

