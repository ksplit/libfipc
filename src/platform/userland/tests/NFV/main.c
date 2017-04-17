/**
 * @File     : main.c
 * @Author   : Abdullah Younis
 * @Copyright: University of Utah
 *
 * This test passes a simulated packet through a series of processes, which
 * represent composed functions. This is done using separate address spaces
 * to isolate the functions.
 *
 * NOTE: This test assumes an x86 architecture.
 *
 * NOTE: This test assumes a computer with four processing units.
 */

#include "../libfipc_test.h"

#define CHANNEL_ORDER ilog2(sizeof(message_t))
#define TRANSACTIONS  1000000

const char* shm_keys[] = { "FIPC_NFV_S0", "FIPC_NFV_S1", "FIPC_NFV_S2" }; 

uint64_t func1 ( uint64_t i )
{
	return i + 1;
}

uint64_t func2 ( uint64_t i )
{
	return i + 2;
}

uint64_t func3 ( uint64_t i )
{
	return i + 3;
}

uint64_t func4 ( uint64_t i )
{
	return i + 4;
}

int main ( void )
{
	///////////// Setup Processes

	// Create 4 processes: P0, P1, P2, P3
	int rank;
	for ( rank = 0; rank < 3; rank++ )
	{
		pid_t child = fork();
		if ( child == 0 )
			break;
	}

	// Pin Px to processor x
	fipc_test_thread_pin_this_process_to_CPU( rank );

	///////////// Setup IPC Mechanism

	fipc_init();

	// Each header is capable of bidirectional communication; however,
	// we are dealing with composed functions, so our messages only
	// move in one direction. Therefore, we use the headers in a unidirectional
	// fashion. Each header's receive buffer talks to the previous process,
	// where each header's transmit buffer talks to the next process.
	//
	// NOTE: In this test, P0 will not receive; likewise, P3 will not send

	header_t*  chan = NULL;
	message_t* tx   = NULL;
	message_t* rx   = NULL;

	if ( rank != 3 )
		fipc_test_shm_create_half_channel( CHANNEL_ORDER, &chan, shm_keys[rank], FIPC_TEST_TRANSMIT );
	if ( rank != 0 )
		fipc_test_shm_create_half_channel( CHANNEL_ORDER, &chan, shm_keys[rank - 1], FIPC_TEST_RECEIVE );

	register uint64_t CACHE_ALIGNED transaction_id;
	register uint64_t CACHE_ALIGNED start;
	register uint64_t CACHE_ALIGNED end;
	register uint64_t* times = malloc( TRANSACTIONS * sizeof( uint64_t ) );
	for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
	{
		start = RDTSC_START();

		if ( rank == 0 )
		{
			fipc_test_blocking_send_start( chan, &tx );
			tx->regs[0] = func1( 0 );
			fipc_send_msg_end ( chan, tx );
		}
		else if ( rank == 1 )
		{
			fipc_test_blocking_recv_start( chan, &rx );
			uint64_t temp = func2( rx->regs[0] );
			fipc_recv_msg_end( chan, rx );

			fipc_test_blocking_send_start( chan, &tx );
			tx->regs[0] = temp;
			fipc_send_msg_end( chan, tx );
		}
		else if ( rank == 2 )
		{
			fipc_test_blocking_recv_start( chan, &rx );
			uint64_t temp = func3( rx->regs[0] );
			fipc_recv_msg_end( chan, rx );

			fipc_test_blocking_send_start( chan, &tx );
			tx->regs[0] = temp;
			fipc_send_msg_end( chan, tx );
		}
		else if ( rank == 3 )
		{
			fipc_test_blocking_recv_start( chan, &rx );
			func4(rx->regs[0]);
			fipc_recv_msg_end( chan, rx );
		}

		end = RDTSCP();

		times[transaction_id] = end - start;
	}

	if ( rank == 0 )
	{
		fipc_test_stat_print_info( times, TRANSACTIONS );

		fipc_test_blocking_send_start( chan, &tx );
		fipc_send_msg_end ( chan, tx );
	}
	else if ( rank == 1 )
	{
		fipc_test_blocking_recv_start( chan, &rx );
		fipc_recv_msg_end( chan, rx );

		fipc_test_stat_print_info( times, TRANSACTIONS );

		fipc_test_blocking_send_start( chan, &tx );
		fipc_send_msg_end( chan, tx );
	}
	else if ( rank == 2 )
	{
		fipc_test_blocking_recv_start( chan, &rx );
		fipc_recv_msg_end( chan, rx );

		fipc_test_stat_print_info( times, TRANSACTIONS );

		fipc_test_blocking_send_start( chan, &tx );
		fipc_send_msg_end( chan, tx );
	}
	else if ( rank == 3 )
	{
		fipc_test_blocking_recv_start( chan, &rx );
		fipc_recv_msg_end( chan, rx );

		fipc_test_stat_print_info( times, TRANSACTIONS );
	}


//	printf( "Hello from process %d.\n", rank );

	//if ( rank != 3 )
	//	fipc_test_shm_unlink( shm_keys[rank] );

	//fipc_test_shm_free_half_channel( chan );

	fipc_fini();
	return 0;
}
