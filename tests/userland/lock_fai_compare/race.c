/**
 * @File   : race.c
 * @Author : Jeonghoon Lee
 *
 * This file is a short test code for testing the latency of spinlock, ticket spinlock, 
 *  mcs lock, and atomic operation fetch-and-increment(FAI).
 * A typical example that makes race condition using pthread.
 */

#include <stdio.h>
#include <stdlib.h>

#ifndef LIBFIPC_TEST
#include "../libfipc_test.h"
#endif

#include "../queue_mpmc_mcslock/mcs.h"
#include "../queue_mpmc_spinlock/spinlock.h"
#include "../queue_mpmc_ticketlock/ticketlock.h"

#ifndef __PTHREAD__
#include <pthread.h>
#endif

volatile int g_cnt = 0;
int g_op;

mcslock* g_mcs;
struct thread_spinlock g_spin;
struct thread_ticketlock g_ticket;

void *counter(void* data)
{
	int i;
    int n = *(int*)data;
    
    qnode node;
   
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

    return 0;
}

int main(int argc, char* argv[])
{
	int n = 10000000;

    if ( argc == 2 ) {
        g_op = atoi( argv[1] );
    }
    else if ( argc == 3 ) {
        g_op = atoi( argv[1] );
        n = atoi( argv[2] );
    }
    else {
        printf("usage: <option> <increment times>\n");
    }

	pthread_t p1, p2;

    if ( g_op == 1 ) {
        printf("option [1] mcs lock, increment: %d\n", n);
        mcs_init_global( &g_mcs );
    }
    else if ( g_op == 2 ) {
        printf("option [2] spin lock, increment: %d\n", n);
        thread_spin_init( &g_spin );
    }
    else if ( g_op == 3 ) {
        printf("option [3] ticket spin lock, increment: %d\n", n);
        thread_ticket_spin_init( &g_ticket );
    }
    else if ( g_op == 4 ) {
        printf("option [4] atomic FAI, increment: %d\n", n);
    }
    else {
        printf("Wrong option\n");
        return 0;
    }   

	pthread_create( &p1, NULL, counter, &n );
	pthread_create( &p2, NULL, counter, &n );

    pthread_join( p1, NULL );
    pthread_join( p2, NULL );

	printf("result: %d\n", g_cnt);

	return 0;
}

