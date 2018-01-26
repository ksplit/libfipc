/**
 * @File     : queue.c
 * @Author   : Abdullah Younis
 */

#include "queue.h"

// Constructor

int init_queue ( queue_t* q )
{
	fipc_test_create_channel( CHANNEL_ORDER, &q->head, &q->tail );

	if ( q->head == NULL || q->tail == NULL )
	{
		pr_err( "%s\n", "Error while creating channel" );
		return -1;
	}

	return SUCCESS;
}

// Destructor

int free_queue ( queue_t* q )
{
	fipc_test_free_channel( CHANNEL_ORDER, q->head, q->tail );
	return SUCCESS;
}

// Enqueue

int enqueue ( queue_t* q, data_t a, data_t b, data_t c, data_t d, data_t e, data_t f, data_t g )
{
	node_t* msg;

	if (fipc_send_msg_start( q->head, &msg ) != 0)
		return NO_MEMORY;

	msg->regs[0] = a;
	msg->regs[1] = b;
	msg->regs[2] = c;
	msg->regs[3] = d;
	msg->regs[4] = e;
	msg->regs[5] = f;
	msg->regs[6] = g;
	fipc_send_msg_end ( q->head, msg );

	return SUCCESS;
}

// Dequeue

int dequeue ( queue_t* q, data_t* a, data_t* b, data_t* c, data_t* d, data_t* e, data_t* f, data_t* g )
{
	node_t* msg;

	if (fipc_recv_msg_start( q->tail, &msg) != 0)
		return EMPTY_COLLECTION;

	*a = msg->regs[0];
	*b = msg->regs[1];
	*c = msg->regs[2];
	*d = msg->regs[3];
	*e = msg->regs[4];
	*f = msg->regs[5];
	*g = msg->regs[6];
	fipc_recv_msg_end( q->tail, msg );

	return SUCCESS;
}
