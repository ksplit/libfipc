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

	return SUCCESS;
}

// Destructor

int free_queue ( queue_t* q )
{
	// STUB
	return SUCCESS;
}

// Enqueue
// We will enqueue *r at the end of *q.

int enqueue ( queue_t* q, request_t* r )
{
	// 1. Ground r.
	r->next = NULL;

	// 2. Make private copy of tail
	request_t* pTail = q->tail;

	// 3. Attempt to change q->tail until success (starvation possible)
	while( !fipc_test_CAS( &q->tail, pTail, r ) );

	// 4. Finish Linking
	if ( pTail != NULL )
		// 4a. Nonempty queue, update previous tail
		pTail->next = r;
	else
		// 4b. Empty queue, reset head
		q->head = r;

	return SUCCESS;
}

// Dequeue
// We will dequeue a node from the front of *q and place it in **r

int dequeue ( queue_t* q, request_t** r )
{
	int finished = 0;

	// 1. Make private copy of head
	request_t* pHead = q->head;

	// 2. If queue isn't empty
	if ( pHead != NULL )
	{
		// 3. Make private copy of tail
		request_t* pTail = q->tail;

		// 4. Attempt to dequeue
		while ( !finished )
		{
			request_t* next = pHead->next;

			// 4a. If queue is empty, finish
			if ( pHead == NULL )
			{
				finished = true;
			}

			// 4b. If queue has one item, update both head and tail
			else if ( pHead == pTail )
			{
				// Try to set tail to null
				finished = fipc_test_CAS( &q->tail, pTail, NULL );

				// If that worked, try to set head to null,
				// unless enqueuer reset it
				if ( finished )
					fipc_test_CAS( &q->head, pHead, NULL );
			}

			// 4c. Multi-item list and no enqueue operation in progress
			else if ( next != NULL )
			{
				finished = fipc_test_CAS( &q->head, pHead, next );
			}

			// 4d. Else last item already being dequeued or intermediate state
			else
			{
				pHead = NULL;
				finished = true;
			}
		}

		// 5. If dequeue succeeded then ground dequeued item
		if ( pHead != NULL )
			pHead->next = NULL;
	}

	// 6. Return dequeued item
	*r = pHead;
	return SUCCESS;
}
