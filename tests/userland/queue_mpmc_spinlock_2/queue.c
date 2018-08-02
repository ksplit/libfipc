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

	thread_spin_init( &(q->H_lock) );
	thread_spin_init( &(q->T_lock) );

	return SUCCESS;
}

int free_queue ( queue_t* q )
{
	return SUCCESS;
}

// Enqueue

int enqueue ( queue_t* q, node_t* node )
{
	thread_spin_lock( &(q->T_lock) );
	node->next = NULL;

	if ( q->tail )
	{
		q->tail->next = node;
		q->tail = node;
	}
	else
	{
		q->head = node;
		q->tail = node;
	}

	thread_spin_unlock( &(q->T_lock) );

	return SUCCESS;
}


// Dequeue

int dequeue ( queue_t* q, uint64_t* data )
{
	node_t* temp;
	node_t* new_head;

	// Acquire Lock, Enter Critical Section
	thread_spin_lock( &(q->H_lock) );
	
	temp     = q->head;
	new_head = q->head->next;

	if ( !new_head )
	{
		thread_spin_unlock( &(q->H_lock) );
		return EMPTY_COLLECTION;
	}

//	*data   = new_head->data;
	*data   = temp->data;
	q->head = new_head;

	// Release Lock, Exit Critical Section
	thread_spin_unlock( &(q->H_lock) );

	return SUCCESS;
}

