/**
* @File     : queue.c
* @Author   : Abdullah Younis
*/

#include "queue.h"

// Constructor

int init_queue(queue_t* q)
{
	q->header.next = NULL;

	q->head = &(q->header);
	q->tail = &(q->header);

//	thread_spin_init(&(q->H_lock));
//	thread_spin_init(&(q->T_lock));

	return SUCCESS;
}

// Destructor

int free_queue(queue_t* q)
{
	// STUB
	return SUCCESS;
}

// Enqueue

int enqueue(queue_t* q, node_t* r)
{
	r->next = NULL;

	// Acquire Lock, Enter Critical Section
//	thread_spin_lock(&(q->T_lock));
	if ( q->tail )
	{	
		q->tail->next = r;
		q->tail = r;
	}
	else
	{
		q->head = r;
		q->tail = r;
	}

	// Release Lock, Exit Critical Section
//	thread_spin_unlock(&(q->T_lock));
	return SUCCESS;
}

// Dequeue

int dequeue(queue_t* q, uint64_t* data)
{
	node_t* temp;
	node_t* new_head;

	// Acquire Lock, Enter Critical Section
//	thread_spin_lock(&(q->H_lock));

	temp = q->head;
	new_head = q->head->next;

	if (new_head == NULL)
	{
//		thread_spin_unlock(&(q->H_lock));
		return EMPTY_COLLECTION;
	}

	*data = new_head->data;
	q->head = new_head;

	// Release Lock, Exit Critical Section
//	thread_spin_unlock(&(q->H_lock));

	return SUCCESS;
}
