/**
 * @File     : libfipc_test.h
 * @Author   : Charles Jacobsen
 * @Author   : Abdullah Younis
 * @Copyright: University of Utah
 *
 * This library contains various helper functions for the several fipc tests.
 *
 * NOTE: This library assumes an x86 architecture.
 */

#ifndef LIBFIPC_TEST_LIBRARY_LOCK
#define LIBFIPC_TEST_LIBRARY_LOCK

#define fipc_test_lfence()    asm volatile ( "lfence" :: )
#define fipc_test_sfence()    asm volatile ( "sfence" :: )
#define fipc_test_mfence()    asm volatile ( "mfence" :: )
#define fipc_test_pause()     asm volatile ( "pause\n": : :"memory" )
#define fipc_test_clflush(X)  asm volatile ( "clflush %0" : "+m" (*(volatile char*)X) )
#define fipc_test_prefetch(X) __builtin_prefetch( (void*)&X )

#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/vmalloc.h>
#include <libfipc.h>

#define NUM_CORES num_online_cpus()

typedef struct task_struct kthread_t;
typedef struct fipc_message cache_line_t;
typedef CACHE_ALIGNED unsigned long long cache_aligned_ull_int_t;

#include "libfipc_test_time.h"
#include "libfipc_test_thread.h"
#include "libfipc_test_stat.h"

/**
 * This function initializes the two headers referenced by h1 and h2 to point
 * to shared memory buffers of size 2^buffer_order
 */
static inline
int fipc_test_create_channel ( size_t buffer_order, header_t** h1, header_t** h2 )
{
	int       error_code = 0;
	void*     buffer1    = NULL;
	void*     buffer2    = NULL;
	header_t* tempH1     = NULL;
	header_t* tempH2     = NULL;
	size_t    page_order = ( buffer_order < PAGE_SHIFT ? 0 : buffer_order - PAGE_SHIFT );

	// (1) Allocate Buffer Pages
	buffer1 = (void*) __get_free_pages( GFP_KERNEL, page_order );

	if ( buffer1 == NULL )
	{
		error_code = -ENOMEM;
		goto fail1;
	}

	buffer2 = (void*) __get_free_pages( GFP_KERNEL, page_order );

	if (  buffer2 == NULL )
	{
		error_code = -ENOMEM;
		goto fail2;
	}

	// (2) Initialize Buffers
	error_code = fipc_prep_buffers( buffer_order, buffer1, buffer2 );

	if ( error_code )
	{
		goto fail3;
	}

	// (3) Allocate Headers
	tempH1 = (header_t*) kmalloc( sizeof( header_t ), GFP_KERNEL );

	if ( tempH1 == NULL )
	{
		error_code = -ENOMEM;
		goto fail4;
	}

	tempH2 = (header_t*) kmalloc( sizeof( header_t ), GFP_KERNEL );

	if ( tempH2 == NULL )
	{
		error_code = -ENOMEM;
		goto fail5;
	}

	// (4) Initialize Headers
	error_code = fipc_ring_channel_init( tempH1, buffer_order, buffer1, buffer2 );

	if ( error_code )
	{
		goto fail6;
	}

	error_code = fipc_ring_channel_init( tempH2, buffer_order, buffer2, buffer1 );

	if ( error_code )
	{
		goto fail7;
	}

	*h1 = tempH1;
	*h2 = tempH2;
	goto success;

fail7:
fail6:
	kfree ( tempH2 );
fail5:
	kfree ( tempH1 );
fail4:
fail3:
	free_pages( (unsigned long) buffer2, page_order );
fail2:
	free_pages( (unsigned long) buffer1, page_order );
fail1:
success:
	return error_code;
}

/**
 * This function will destroy the channel and handle memory deallocation.
 */
static inline
void fipc_test_free_channel ( size_t buffer_order, header_t* h1, header_t* h2 )
{
	size_t page_order = ( buffer_order < PAGE_SHIFT ? 0 : buffer_order - PAGE_SHIFT );

	// Free Buffers
	free_pages( (unsigned long)h1->tx.buffer, page_order );
	free_pages( (unsigned long)h1->rx.buffer, page_order );

	// Free Headers
	kfree( h1 );
	kfree( h2 );
}

/**
 * This function will block until a message is received and stored in out.
 */
static inline
int fipc_test_blocking_recv_start ( header_t* channel, message_t** out )
{
	int ret;

	while ( 1 )
	{
		// Poll until we get a message or error
		ret = fipc_recv_msg_start( channel, out );

		if ( !ret || ret != -EWOULDBLOCK )
		{
			return ret;
		}
		
		fipc_test_pause();
	}

	return 0;
}

/**
 * This function will block until a message slot is available and stored in out.
 */
static inline
int fipc_test_blocking_send_start ( header_t* channel, message_t** out )
{
	int ret;

	while ( 1 )
	{
		// Poll until we get a free slot or error
		ret = fipc_send_msg_start( channel, out );

		if ( !ret || ret != -EWOULDBLOCK )
		{
			return ret;
		}

		fipc_test_pause();
	}

	return 0;
}

/**
 * This function will block until a long message is available and stored in out.
 */
static inline
int fipc_test_blocking_long_send_start ( header_t* channel, message_t** out, uint16_t len )
{
	int ret;

	while ( 1 )
	{
		// Poll until we get a free slot or error
		ret = fipc_send_long_msg_start( channel, out, len );

		if ( !ret || ret != -EWOULDBLOCK )
		{
			return ret;
		}

		fipc_test_pause();
	}

	return 0;
}

#endif
