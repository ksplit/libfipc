/**
 * @File     : queue.c
 * @Author   : Abdullah Younis
 */

#include "queue.h"

// Constructor

int init_queue ( queue_t* q )
{
	int i;

	q->node_table = (node_t*) vmalloc( PREALLOCATED_NODES*sizeof(node_t) );

	for ( i = 0; i < PREALLOCATED_NODES; ++i )
	{
		q->node_table[i].data = 0;
		q->node_table[i].next = NULL;
	}

	q->physical_size = (PREALLOCATED_NODES-1);
	q->logical_size  = 0;

	q->head = q->tail = &q->node_table[PREALLOCATED_NODES-1];

	spin_lock_init( &q->queue_lock );

	return SUCCESS;
}

// Destructor

int free_queue ( queue_t* q )
{
	vfree ( q->node_table );
	return SUCCESS;
}

// Enqueue

int enqueue ( queue_t* q, uint64_t data )
{
	node_t* new_node;

	// Acquire Lock, Enter Critical Section
	spin_lock( &q->queue_lock );

	if ( q->logical_size >= q->physical_size )
	{
		spin_unlock( &q->queue_lock );
		return NO_MEMORY;
	}

	new_node = &q->node_table[ q->logical_size ];
	new_node->data = data;

	q->tail->next = new_node;
	q->tail       = new_node;

	q->logical_size++;

	// Release Lock, Exit Critical Section
	spin_unlock( &q->queue_lock );
	return SUCCESS;
}

// Dequeue

int dequeue ( queue_t* q, uint64_t* data )
{
	node_t* temp;

	// Acquire Lock, Enter Critical Section
	spin_lock( &q->queue_lock );

	if ( q->logical_size == 0 )
	{
		spin_unlock( &q->queue_lock );
		return EMPTY_COLLECTION;
	}

	temp          = q->head->next;
	q->head->next = q->head->next->next;

	*data = temp->data;
	// delete temp (unnecessary since preallocated)

	q->logical_size--;

	if ( q->logical_size == 0 )
		q->tail = q->head;

	// Release Lock, Exit Critical Section
	spin_unlock( &q->queue_lock );
	return SUCCESS;
}
