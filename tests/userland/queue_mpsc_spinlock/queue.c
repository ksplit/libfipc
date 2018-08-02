/**
 * @File     : queue.c
 * @Author   : Abdullah Younis
 */

#include "queue.h"

#define pr_err printf
// Constructor

int init_queue ( queue_t* q )
{
	q->header.next = NULL;

	q->head = &(q->header);
	q->tail = &(q->header);

    	thread_spin_init(&(q->H_lock));
	thread_spin_init(&(q->T_lock));

	return SUCCESS;
}

// Destructor

int free_queue ( queue_t* q )
{
	// STUB
	return SUCCESS;
}

// Enqueue
/*
int enqueue_blk ( queue_t* q, request_t* r )
{
	message_t* msg;

	fipc_test_blocking_send_start(q->head, &msg );
	msg->regs[0] = (uint64_t)node;
	fipc_send_msg_end ( q->head, msg );

	return SUCCESS;
}

// Dequeue

int dequeue_blk ( queue_t* q, uint64_t* data )
{
	message_t* msg;

	fipc_test_blocking_recv_start(q->tail, &msg);
	*n = (node_t*)msg->regs[0];
	fipc_recv_msg_end( q->tail, msg );
	
	return SUCCESS;
}
*/
int enqueue ( queue_t* q, request_t* r )
{
	r->next = NULL;
	thread_spin_lock( &(q->T_lock) );
//	printf("lock :: %d\n", q->T_lock.locked);
	if(q->tail) 
	{
	        q->tail->next = r;
        	q->tail = r;
    	}
	else
	{
        	q->head = r;
        	q->tail = r;
    	}

	thread_spin_unlock( &(q->T_lock) );
//	printf("unlock :: %d\n", q->T_lock.locked);
	
	return SUCCESS;
}

// Dequeue

int dequeue ( queue_t* q, uint64_t* data )
{
	request_t* temp;
	request_t* new_head;

	// Acquire Lock, Enter Critical Section
	thread_spin_lock( &(q->H_lock) );
	
	temp     = q->head;
	new_head = q->head->next;

	if ( new_head == NULL )
	{
		thread_spin_unlock( &(q->H_lock) );
		return EMPTY_COLLECTION;
	}

	*data   = new_head->data;
	q->head = new_head;

	// Release Lock, Exit Critical Section
	thread_spin_unlock( &(q->H_lock) );
/*	
	temp = q->head;

	if( !temp )
	{
		printf("temp == NULL\n");
		thread_spin_unlock( &(q->H_lock) );
		return EMPTY_COLLECTION;
	}
	if(!temp->data)
		printf("eng!!!\n");
		
	*data = temp->data;
	new_head = q->head->next;
	
	if( q->head->next )
        {
		new_head = q->head->next;
		
		printf("y?\n");
	        thread_spin_unlock( &(q->H_lock) );
                return EMPTY_COLLECTION;
        
	}
	else
	{
		new_head = NULL;
	}

	*data = temp->data;
	q->head = new_head;
	
	// Release Lock, Exit Critical Section
	thread_spin_unlock( &(q->H_lock) );
*/
	return SUCCESS;
}

