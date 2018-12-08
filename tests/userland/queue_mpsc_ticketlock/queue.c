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

	thread_ticket_spin_init( &q->H_lock );
	thread_ticket_spin_init( &q->T_lock );

	return SUCCESS;
}

int free_queue ( queue_t* q )
{
	return SUCCESS;
}

// Enqueue
int enqueue ( queue_t* q, node_t* r )
{
	r->next = NULL;
	
	thread_ticket_spin_lock( &q->T_lock );

	q->tail->next = r;
	q->tail = r;

	thread_ticket_spin_unlock( &q->T_lock );

	return SUCCESS;
}


// Dequeue

int dequeue ( queue_t* q, uint64_t* data )
{
	node_t* temp, * new_head;

	thread_ticket_spin_lock( &q->H_lock );

	temp	 = q->head;
	new_head = temp->next;

	if ( new_head == NULL )
	{
		thread_ticket_spin_unlock( &q->H_lock );
		return EMPTY_COLLECTION;
	}

	*data 	= new_head->data;
	q->head = new_head;

	thread_ticket_spin_unlock( &q->H_lock );

	return SUCCESS;
}

