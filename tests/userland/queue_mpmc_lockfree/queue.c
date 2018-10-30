/**
 * @File     : queue.c
 * @Author   : Abdullah Younis
 */

#include "queue.h"

#define pr_err printf

int init_queue ( queue_t* q )
{
	q->header.next = NULL;
	q->head = &q->header;
	q->tail = &q->header;

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
	node_t* tail = NULL;
	node_t* next = NULL;
	r->next = NULL;
	
	while ( 1 )
	{
		tail = q->tail;
		next = tail->next;
		
		if ( tail == q->tail )
		{
			if ( next == NULL )
			{
				if ( fipc_test_CAS( &tail->next, next, r ) )
					break;
			}
			else
			{
				fipc_test_CAS( &q->tail, tail, next );
			}
		}
	}

	fipc_test_CAS(&q->tail, tail, r);

	return SUCCESS;
}

// Dequeue
int dequeue ( queue_t* q, uint64_t* data )
{
	node_t* head = NULL;
	node_t* tail = NULL;
	node_t* next = NULL;
	
	while ( 1 )
	{
		head = q->head;
		tail = q->tail;
		next = head->next;
		if ( head == q->head )
		{
		 	if ( head == tail )
			{
				if(next == NULL)
					return EMPTY_COLLECTION;

				fipc_test_CAS(&q->tail, tail, next);
			}
			else
			{
				*data = next->data;
				fipc_test_mfence();

				if( fipc_test_CAS(&q->head, head, next) )
					break;
			}
		}
	}

	return SUCCESS;
}

