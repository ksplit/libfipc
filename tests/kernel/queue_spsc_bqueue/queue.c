/**
 * @File     : queue.c
 * @Author   : 
 *
 * CITE: https://github.com/olibre/B-Queue/blob/master/
 */

#include "queue.h"

// Constructor

int init_queue ( queue_t* q )
{
	memset(q, 0, sizeof(struct queue_t));
	q->batch_history = BATCH_SIZE;

	return SUCCESS;
}

// Destructor

int free_queue ( queue_t* q )
{
	// STUB
	return SUCCESS;
}

// Enqueue

int enqueue ( queue_t* q, data_t d )
{
	uint64_t tmp_head;

	if( q->head == q->batch_head )
	{
		tmp_head = q->head + BATCH_SIZE;

		if ( tmp_head >= QUEUE_SIZE )
			tmp_head = 0;

		if ( q->data[tmp_head] )
		{
			fipc_test_time_wait_ticks(CONGESTION_PENALTY);
			return BUFFER_FULL;
		}

		q->batch_head = tmp_head;
	}

	q->data[q->head] = d;
	q->head++;

	if ( q->head >= QUEUE_SIZE )
	{
		q->head = 0;
	}

	return SUCCESS;
}

static inline
int backtracking ( queue_t * q )
{
	uint64_t tmp_tail = q->tail + BATCH_SIZE;

	if ( tmp_tail >= QUEUE_SIZE )
	{
		tmp_tail = 0;
		if (q->batch_history < BATCH_SIZE)
		{
			q->batch_history = 
				(BATCH_SIZE < (q->batch_history + BATCH_INCREAMENT))? 
				BATCH_SIZE : (q->batch_history + BATCH_INCREAMENT);
		}
	}


	unsigned long batch_size = q->batch_history;

	while (!(q->data[tmp_tail]))
	{
		fipc_test_time_wait_ticks(CONGESTION_PENALTY);

		batch_size = batch_size >> 1;
		if( batch_size >= 0 )
		{
			tmp_tail = q->tail + batch_size;
			if (tmp_tail >= QUEUE_SIZE)
				tmp_tail = 0;
		}
		else
			return -1;
	}

	q->batch_history = batch_size;



	if ( tmp_tail == q->tail )
	{
		tmp_tail = (tmp_tail + 1) >= QUEUE_SIZE ?
			0 : tmp_tail + 1;
	}
	q->batch_tail = tmp_tail;

	return 0;
}

// Dequeue

int dequeue ( queue_t* q, data_t* d )
{
	if( q->tail == q->batch_tail )
	{
		if ( backtracking(q) != 0 )
			return BUFFER_EMPTY;
	}

	*d = q->data[q->tail];

	q->data[q->tail] = 0;
	q->tail++;
	if ( q->tail >= QUEUE_SIZE )
		q->tail = 0;

	return SUCCESS;
}
