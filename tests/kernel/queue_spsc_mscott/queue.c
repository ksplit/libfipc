/**
 * @File     : queue.c
 * @Author   : Abdullah Younis
 */

#include "queue.h"

// Constructor

int init_queue ( queue_t* q )
{
	q->head = NULL;
	q->tail = NULL;

	spin_lock_init( &q->queue_lock );

	return SUCCESS;
}

// Destructor

int free_queue ( queue_t* q )
{
	// STUB
	return SUCCESS;
}

// Enqueue

int enqueue ( queue_t* q, request_t* r )
{
	// Acquire Lock, Enter Critical Section
	spin_lock( &q->queue_lock );

	if ( q->tail == NULL )
	{
		q->head = q->tail = r;
	}
	else
	{
		q->tail->next = r;
		q->tail       = r;
	}

	// Release Lock, Exit Critical Section
	spin_unlock( &q->queue_lock );
	return SUCCESS;
}

// Dequeue

int dequeue ( queue_t* q, request_t** r )
{
	// Acquire Lock, Enter Critical Section
	spin_lock( &q->queue_lock );

	if ( q->head == NULL )
	{
		spin_unlock( &q->queue_lock );
		return EMPTY_COLLECTION;
	}

	*r      = q->head;
	q->head = q->head->next;

	if ( q->head == NULL )
		q->tail = q->head;

	(*r)->next = NULL;

	// Release Lock, Exit Critical Section
	spin_unlock( &q->queue_lock );
	return SUCCESS;
}
