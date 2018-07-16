/**
 * @File     : queue.c
 * @Author   : Abdullah Younis
 */

#include "queue.h"

// Constructor

int init_queue ( queue_t* q )
{
	q->header.next = NULL;

	q->head = &(q->header);
	q->tail = &(q->header);

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
	r->next = NULL;
	
	// Acquire Lock, Enter Critical Section
	spinlock_lock(&T_lock, &limit);

	q->tail->next = r;
	q->tail       = r;

	// Release Lock, Exit Critical Section
	spinlock_unlock(&T_lock, &limit);
	return SUCCESS;
}

// Dequeue

int dequeue ( queue_t* q, uint64_t* data )
{
	request_t* temp;
	request_t* new_head;
	unit32_t limit = 1;

	// Acquire Lock, Enter Critical Section
	spinlock_lock(&H_lock, &limit);

	temp     = q->head;
	new_head = q->head->next;

	if ( new_head == NULL )
	{
		spinlock_lock(&H_lock, &limit);
		return EMPTY_COLLECTION;
	}

	*data   = new_head->data;
	q->head = new_head;

	// Release Lock, Exit Critical Section
	spinlock_unlock(&H_lock, &limit);

	return SUCCESS;
}
