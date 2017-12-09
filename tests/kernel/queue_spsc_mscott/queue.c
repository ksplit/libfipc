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

int enqueue ( queue_t* q, node_t* r )
{
	r->next = NULL;

	q->tail->next = r;
	q->tail       = r;

	return SUCCESS;
}

// Dequeue

int dequeue ( queue_t* q, data_t* data )
{
	node_t* temp;
	node_t* new_head;

	temp     = q->head;
	new_head = q->head->next;

	if ( new_head == NULL )
		return EMPTY_COLLECTION;

	*data   = new_head->data;
	q->head = new_head;

	return SUCCESS;
}
