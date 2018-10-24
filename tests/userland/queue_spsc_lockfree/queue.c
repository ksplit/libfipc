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
	node_t* tail;
	node_t* next;
	r->next = NULL;
	while (1)
	{
		tail = q->tail;
		next = tail->next;
		
		if (tail == q->tail)
		{
			if (next == NULL)
			{
				//pointer_t *p = malloc(sizeof(pointer_t));
				//p->ptr = r;
				//p->count = next.count+1;
				if ( fipc_test_CAS( &tail->next, next, r ) )
					break;
				//else
					//free(p);
			}
			else
			{
				//pointer_t *p = malloc(sizeof(pointer_t));
				//p->ptr = next.ptr;
				//p->count = tail.count+1;
				if( !fipc_test_CAS( &q->tail, tail, next ) )
					;
					//free(p);
			}
		}
	}
	//pointer_t *p = malloc(sizeof(pointer_t));
	//p->ptr = r;
	//p->count = tail.count + 1;
	if( !fipc_test_CAS(&q->tail, tail, r))
		;
		//free(p);

	return SUCCESS;
}

// Dequeue
int dequeue ( queue_t* q, uint64_t* data )
{
	node_t* head;
	node_t* tail;
	node_t* next;
	
	while (1)
	{
		head = q->head;
		tail = q->tail;
		next = head->next;
		if (head == q->head)
		{
		 	if (head == tail)
			{
				if(next == NULL)
					return EMPTY_COLLECTION;

				//pointer_t *p = malloc(sizeof(pointer_t));
				//p->ptr = next.ptr;
				//p->count = tail.count+1;
				if (!fipc_test_CAS(&q->tail, tail, next))
					;
					//free(p);
			}
			else
			{
				*data = next->data;
				fipc_test_mfence();
				//pointer_t *p = malloc(sizeof(pointer_t));
				//p->ptr = next.ptr;
				//p->count = head.count+1;
				if(fipc_test_CAS(&q->head, head, next))
					break;
				else
					;
					//free(p);
			}
		}
	}
	//free(head.ptr);
	return SUCCESS;
}

