/**
 * @File   : race.c
 * @Author : Jeonghoon Lee
 *
 * This file is a short test code for testing the validity of mcs lock.
 * A typical example that makes race condition using pthread.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "mcs.h"

int cnt = 0;
pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
mcslock* g_mcs;

void *counter(void* data)
{
	int i;
    int n = *(int*)data;
    
    qnode node;
    mcs_init_local( &node );
    
	for ( i = 0; i < n; ++i)
	{
        mcs_lock( &g_mcs, &node );
        //pthread_mutex_lock( &g_lock );
		++cnt;
        //pthread_mutex_unlock( &g_lock );
        mcs_unlock( &g_mcs, &node );
	}
}

int main(int argc, char* argv[])
{
    int i;
	int n = 100;

    if ( argc == 2 ) {
        n = atoi( argv[1] );
    }

	pthread_t p1, p2;

//    pthread_mutex_init( &g_mcs, NULL);
    mcs_init_global( &g_mcs );

	pthread_create( &p1, NULL, counter, &n );
	pthread_create( &p2, NULL, counter, &n );

    pthread_join( p1, NULL );
    pthread_join( p2, NULL );

	printf("result: %d\n", cnt);

//    pthread_mutex_destroy( &g_lock );

	return 0;
}
