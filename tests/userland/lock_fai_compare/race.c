/**
 * @File   : race.c
 * @Author : Jeonghoon Lee
 * @Author : Minjun Cha
 * This file is a short test code for testing the latency of spinlock, ticket spinlock, 
 *  mcs lock, and atomic operation fetch-and-increment(FAI).
 * A typical example that makes race condition using pthread.
 */

#include <stdio.h>
#include <stdlib.h>

#ifndef LIBFIPC_TEST
#include "../libfipc_test.h"
#include "../libfipc_test_time.h"
#endif

#include "../queue_mpmc_mcslock/mcs.h"
#include "../queue_mpmc_spinlock/spinlock.h"
#include "../queue_mpmc_ticketlock/ticketlock.h"

#ifndef __PTHREAD__
#include <pthread.h>
#endif

volatile int g_cnt = 0;
int g_op;
int thread_count = 1;
long long sum_time = 0;
int ready_threads = 0;

mcslock* g_mcs;
struct thread_spinlock g_spin;
struct thread_ticketlock g_ticket;
/*
static uint8_t cpus[32] = { 0,4,8,12,16,20,24,28,
			    1,5,9,13,17,21,25,29,
			    2,6,10,14,18,22,26,30,
			    3,7,11,15,19,23,27,31 };
*/
static uint8_t cpus[64] = { 0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,
			    1,5,9,13,17,21,25,29,33,37,41,45,49,53,57,61,
			    2,6,10,14,18,22,26,30,34,38,42,46,50,54,58,62,
			    3,7,11,15,19,23,27,31,35,39,43,47,51,55,59,63 };


void *counter(void* data)
{
	int i;	
	int n = *(int*)data;
	qnode node;
	int cnt = 0;

	long long start, end;

	fipc_test_FAI( ready_threads );

	while ( ready_threads < thread_count ) {
		fipc_test_pause();
	}

	start = RDTSC_START();

	if ( g_op == 1 ) {
		//for ( i = 0; i < n; ++i )
		while ( g_cnt < n )
		{
			mcs_lock( &g_mcs, &node );
			++g_cnt;
			++cnt;
			mcs_unlock( &g_mcs, &node );
		}
	}
	else if ( g_op == 2 ) {
//		for ( i = 0; i < n; ++i )
		while ( g_cnt < n )
		{
			thread_spin_lock( &g_spin );
			++g_cnt;
			++cnt;
			thread_spin_unlock( &g_spin );
		}
	} 
	else if ( g_op == 3 ) {
//		for ( i = 0; i < n; ++i )
		while ( g_cnt < n )
		{
			thread_ticket_spin_lock( &g_ticket );
			++g_cnt;
			++cnt;
			thread_ticket_spin_unlock( &g_ticket );
		}
	}
	else if ( g_op == 4 ) {
		//for ( i = 0; i < n; ++i )
		while ( g_cnt < n )
		{
			fipc_test_FAI( g_cnt );
			++cnt;
		}
	}

	end = RDTSCP();
//	__sync_fetch_and_add( &sum_time, (end - start) / n );
	__sync_fetch_and_add( &sum_time, end - start );
//	printf("Cycle per n : %lld, Total: %d, N: %d\n", (end - start) / cnt , g_cnt, cnt);
	return 0;
}

int main(int argc, char* argv[])
{
	int n = 10000000;
	int i;

	if ( argc == 2 ) {
		g_op = atoi( argv[1] );
	}
	else if ( argc == 3 ) {
		g_op = atoi( argv[1] );
		n = atoi( argv[2] );
	}
	else if ( argc == 4 ) {
		g_op = atoi (argv[1] );
		n = atoi( argv[2] );
		thread_count = atoi( argv[3] );
	}
	else {
		printf("usage: <option> <increment times>\n");
	}

	if ( g_op == 1 ) {
		printf("Option [1] MCS lock\n");
		printf(">>>>> Number of Threads : %d, Increment: %d\n", thread_count, n);
		mcs_init_global( &g_mcs );
	}
	else if ( g_op == 2 ) {
		printf("Option [2] Spin lock\n");
		printf(">>>>> Number of Threads : %d, Increment: %d\n", thread_count, n);
		thread_spin_init( &g_spin );
	}
	else if ( g_op == 3 ) {
		printf("Option [3] Ticket lock\n");
		printf(">>>>> Number of Threads : %d, Increment: %d\n", thread_count, n);
		thread_ticket_spin_init( &g_ticket );
	}
	else if ( g_op == 4 ) {
		printf("Option [4] FAI\n");
		printf(">>>>> Number of Threads : %d, Increment: %d\n", thread_count, n);
	}
	else {
		printf("Wrong option\n");
		return 0;
	}   

	pthread_t *p[thread_count];

	for( i = 0; i < thread_count; ++i )
//		pthread_create( &p[i], NULL, counter, &n );
		p[i] = fipc_test_thread_spawn_on_CPU( counter, &n, cpus[i] );

	for( i = 0; i < thread_count; ++i )
//		pthread_join ( p[i], NULL );
		fipc_test_thread_wait_for_thread( p[i] );

//	printf(">>>>> Average time : %llu, Final result count : %d \n\n", sum_time / thread_count, g_cnt);
	printf(">>>>> Average time : %llu, Final result count : %d \n\n", sum_time / g_cnt, g_cnt);

	return 0;
}
