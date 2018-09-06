/**
 * @File     : queue.c
 * @Author   : Abdullah Younis
 */

#include "queue.h"

#define pr_err printf

int init_queue ( queue_t* q )
{
	q->header.next = NULL;

	q->head = NULL;
	q->tail = NULL;

    	mcs_init_global( &(q->lock) );

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
	qnode I;
	mcs_init_local( &I );

	r->next = NULL;

	mcs_lock( &(q->lock), &I );

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

	mcs_unlock( &(q->lock), &I );

	
	return SUCCESS;
}
// Dequeue
int dequeue ( queue_t* q, node_t* value )
{
	qnode I;
	mcs_init_local( &I );
        mcs_lock( &(q->lock), &I );

        node_t* tmp = q->head;
        
	if ( !tmp )
        {
        	mcs_unlock( &(q->lock), &I );
		printf("[dequeue] tmp null. release\n");
                return EMPTY_COLLECTION;
        }

        value->data = tmp->data;
        
	if ( q->head == q->tail )
        {
                q->tail = NULL;
        }
        q->head = tmp->next;

       	mcs_unlock( &(q->lock), &I );

        return SUCCESS;
}


