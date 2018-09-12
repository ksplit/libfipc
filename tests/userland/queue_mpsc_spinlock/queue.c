/**
 * @File     : queue.c
 * @Author   : Abdullah Younis
 */

#include "queue.h"

#define pr_err printf

int init_queue ( queue_t* q )
{
	q->head = NULL;
	q->tail = NULL;

    	thread_spin_init(&(q->spin_lock));

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
	thread_spin_lock( &(q->spin_lock) );

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

	thread_spin_unlock( &(q->spin_lock) );
	
	return SUCCESS;
}

// Dequeue
int dequeue ( queue_t* q, uint64_t* data )
{
        thread_spin_lock( &(q->spin_lock) );

        node_t* temp = q->head;
        
	if ( !temp )
        {
        	thread_spin_unlock( &(q->spin_lock) );
                return EMPTY_COLLECTION;
        }

        *data = temp->data;
        
	if ( q->head == q->tail )
        {
                q->tail = NULL;
        }
        q->head = temp->next;

       	thread_spin_unlock( &(q->spin_lock) );

        return SUCCESS;
}
