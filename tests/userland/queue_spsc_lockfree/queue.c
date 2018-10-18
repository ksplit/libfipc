/**
 * @File     : queue.c
 * @Author   : Abdullah Younis
 */

#include "queue.h"

#define pr_err printf

int init_queue ( queue_t* q )
{
	q->header.next.ptr = NULL;
	q->head.ptr = &q->header;
	q->tail.ptr = &q->header;

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
	pointer_t tail;
	pointer_t next;
	r->next.ptr = NULL;
	while(1)
	{
		tail = q->tail;
		next = tail.ptr->next;
		
		if (tail.ptr == q->tail.ptr && tail.count == q->tail.count)
		{
			if (next.ptr == NULL)
			{
				pointer_t *p = malloc(sizeof(pointer_t));
				p->ptr = r;
				p->count = next.count+1;
				if (fast_ipc_CAS(&tail.ptr->next, next, *p))
					break;
				else
					free(p);
			}
			else
			{
				pointer_t *p = malloc(sizeof(pointer_t));
				p->ptr = next.ptr;
				p->count = tail.count+1;
				if(! fast_ipc_CAS(&q->tail, tail, *p))
					free(p);
			}
		}
	}
	pointer_t *p = malloc(sizeof(pointer_t));
	p->ptr = r;
	p->count = tail.count + 1;
	if( !fast_ipc_CAS(&q->tail, tail, *p))
		free(p);
}

// Dequeue
int dequeue ( queue_t* q, uint64_t* data )
{
	pointer_t head;
	pointer_t tail;
	pointer_t next;
	
	while(1)
	{
		head = q->head;
		tail = q->tail;
		next = head.ptr->next;
		if(head.ptr == q->head.ptr && head.count == q->head.count)
		{
		 	if(head.ptr == tail.ptr)
			{
				if(next.ptr == NULL)
					return EMPTY_COLLECTION;

				pointer_t *p = malloc(sizeof(pointer_t));
				p->ptr = next.ptr;
				p->count = tail.count+1;
				if(!fast_ipc_CAS(&q->tail, tail, *p))
					free(p);
			}
			else
			{
				pointer_t *p = malloc(sizeof(pointer_t));
				p->ptr = next.ptr;
				p->count = head.count+1;
				if(fast_ipc_CAS(&q->head, head, *p))
					break;
				else
					free(p);
			}
		}
	}
	free(head.ptr);
        return SUCCESS;
}

