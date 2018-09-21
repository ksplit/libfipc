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

mcslock* g_mcs;
struct thread_spinlock g_spin;
struct thread_ticketlock g_ticket;

void *counter(void* data)
{
	int i;	
	int n = *(int*)data;
    	qnode node;

	long long start, end;

	start = RDTSC_START();
   
    if ( g_op == 1 ) {
    	for ( i = 0; i < n; ++i )
    	{
            mcs_lock( &g_mcs, &node );
		    ++g_cnt;
            mcs_unlock( &g_mcs, &node );
	}
    }
    else if ( g_op == 2 ) {
	for ( i = 0; i < n; ++i )
    	{
            thread_spin_lock( &g_spin );
		    ++g_cnt;
            thread_spin_unlock( &g_spin );
    	}
    } 
    else if ( g_op == 3 ) {
    	for ( i = 0; i < n; ++i )
    	{
            thread_ticket_spin_lock( &g_ticket );
    		++g_cnt;
            thread_ticket_spin_unlock( &g_ticket );
    	}
    }
    else if ( g_op == 4 ) {
    	for ( i = 0; i < n; ++i )
	{
            fipc_test_FAI( g_cnt );
	}
    }

	end = RDTSCP();
	sum_time += (end - start) / n;
	printf("Cycle per n : %lld, Total: %d, N: %d\n", (end - start) / n , g_cnt, n);
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

	pthread_t p[thread_count];
	
	for( i = 0; i < thread_count; ++i )
		pthread_create( &p[i], NULL, counter, &n );

	for( i = 0; i < thread_count; ++i )
    		pthread_join ( p[i], NULL ) ;

	printf(">>>>> Average time : %llu, Final result count : %d \n\n", sum_time / thread_count, g_cnt);

	return 0;
}
