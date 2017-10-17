/**
 * @File     : queue.c
 * @Author   : Abdullah Younis
 */

#include "queue.h"

// Constructor

int init_queue ( queue_t* q )
{
	int i;

	q->node_table = (node_t*) kmalloc( PREALLOCATED_NODES*sizeof(node_t), GFP_KERNEL );

	for ( i = 0; i < PREALLOCATED_NODES; ++i )
	{
		q->node_table[i].data = 0;
		q->node_table[i].next = NULL;
	}

	q->physical_size = (PREALLOCATED_NODES-1);
	q->logical_size  = 0;

	q->head = q->tail = &q->node_table[PREALLOCATED_NODES-1];

	spin_lock_init( &q->queue_lock );
	spin_lock_init( &q->node_table_lock );

	return SUCCESS;
}

// Destructor

int free_queue ( queue_t* q )
{
	kfree ( q->node_table );
	return SUCCESS;
}

// Allocate a free node from node table

int alloc_request ( queue_t* q, request_t** r )
{
	// Acquire Lock, Enter Critical Section
	spin_lock( &q->node_table_lock );

	if ( q->logical_size >= q->physical_size )
	{
		spin_unlock( &q->node_table_lock );
		return NO_MEMORY;
	}

	*r = (request_t*) &q->node_table[q->logical_size];

	q->logical_size++;

	// Release Lock, Exit Critical Section
	spin_unlock( &q->node_table_lock );
	return SUCCESS;
}

// Enqueue

int enqueue ( queue_t* q, request_t* r )
{
	// Acquire Lock, Enter Critical Section
	spin_lock( &q->queue_lock );

	q->tail->next = r;
	q->tail       = r;

	// Release Lock, Exit Critical Section
	spin_unlock( &q->queue_lock );
	return SUCCESS;
}

// Dequeue

int dequeue ( queue_t* q, request_t** r )
{
	// Acquire Lock, Enter Critical Section
	spin_lock( &q->queue_lock );

	if ( q->head->next == NULL )
	{
		spin_unlock( &q->queue_lock );
		return EMPTY_COLLECTION;
	}

	*r            = q->head->next;
	q->head->next = q->head->next->next;

	if ( q->head->next == NULL )
		q->tail = q->head;

	// Release Lock, Exit Critical Section
	spin_unlock( &q->queue_lock );
	return SUCCESS;
}
