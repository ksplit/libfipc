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

#define CHANNEL_ORDER  ilog2(sizeof(message_t))
#define TRANSACTIONS   1000000
#define NUM_PROCESSORS 3

const char* shm_keysF[] = { "FIPC_NFV_S0_F", "FIPC_NFV_S1_F", "FIPC_NFV_S2_F" }; 
const char* shm_keysB[] = { "FIPC_NFV_S0_B", "FIPC_NFV_S1_B", "FIPC_NFV_S2_B" }; 

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
	// Create n processes: P0, P1, P2, ..., Pn-1
	int rank;
	for ( rank = 0; rank < NUM_PROCESSORS - 1; rank++ )
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
	// For time synchronization we need a reversed channel.
	// The channel for communication in the forward direction is the forw channel.
	// The channel for communication in the reverse direction is the back channel.
	//
	// NOTE: In this test, P0 will not receive; likewise, Pn-1 will not send

	header_t*  forw = NULL;
	header_t*  back = NULL;

	message_t* txF  = NULL;
	message_t* rxF  = NULL;
	message_t* txB  = NULL;
	message_t* rxB  = NULL;

	if ( rank != NUM_PROCESSORS-1 )
	{
		fipc_test_shm_create_half_channel( CHANNEL_ORDER, &forw, shm_keysF[rank], FIPC_TEST_TRANSMIT );
		fipc_test_shm_create_half_channel( CHANNEL_ORDER, &back, shm_keysB[rank], FIPC_TEST_RECEIVE );
	}
	if ( rank != 0 )
	{
		fipc_test_shm_create_half_channel( CHANNEL_ORDER, &forw, shm_keysF[rank - 1], FIPC_TEST_RECEIVE );
		fipc_test_shm_create_half_channel( CHANNEL_ORDER, &back, shm_keysB[rank - 1], FIPC_TEST_TRANSMIT );
	}

	///////////// Time Synchronization

	register uint64_t CACHE_ALIGNED transaction_id;
	register uint64_t CACHE_ALIGNED start;
	register uint64_t CACHE_ALIGNED end;
	register uint64_t CACHE_ALIGNED offset; // Represents the difference in time between P0 and PN

	//register uint64_t* times1 = malloc( TRANSACTIONS * sizeof( uint64_t ) );
	//register uint64_t* times2 = malloc( TRANSACTIONS * sizeof( uint64_t ) );
	//register uint64_t* times3 = malloc( TRANSACTIONS * sizeof( uint64_t ) );

	if ( rank == 0 )
	{
		start = RDTSC_START();
		for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
		{
			fipc_test_blocking_send_start( forw, &txF );
			//txF->regs[0] = RDTSC_START();
			fipc_send_msg_end ( forw, txF );

			fipc_test_blocking_recv_start( back, &rxB );
			//uint64_t timestamp = RDTSC_START();
			//times1[transaction_id] = ( timestamp - rxB->regs[0] ) >> 1;
			//times2[transaction_id] = ( rxB->regs[1] - times1[transaction_id] ) - rxB->regs[0];
			//times3[transaction_id] = ( rxB->regs[1] - rxB->regs[0] - times2[transaction_id] );
			fipc_recv_msg_end( back, rxB );
		}
		end = RDTSCP();

		uint64_t t_of_h = (end - start)/(2*transaction_id);
		printf("%lu\n", t_of_h);

		fipc_test_blocking_recv_start( back, &rxB );
		offset = rxB->regs[0] + t_of_h - end;
		fipc_recv_msg_end( back, rxB );
		//fipc_test_stat_print_info( times1, TRANSACTIONS );
		//fipc_test_stat_print_info( times2, TRANSACTIONS );
		//fipc_test_stat_print_info( times3, TRANSACTIONS );
	}
	else if ( rank == NUM_PROCESSORS - 1 )
	{
		for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
		{
			fipc_test_blocking_recv_start( forw, &rxF );
			//uint64_t temp = rxF->regs[0];
			fipc_recv_msg_end( forw, rxF );

			fipc_test_blocking_send_start( back, &txB );
			//txB->regs[0] = temp;
			//txB->regs[1] = RDTSC_START();
			fipc_send_msg_end( back, txB );
		}
		end = RDTSCP();
		fipc_test_blocking_send_start( back, &txB );
		txB->regs[0] = end;
		fipc_send_msg_end( back, txB );
	}
	else
	{
		for ( transaction_id = 0; transaction_id < TRANSACTIONS; transaction_id++ )
		{
			//message_t temp;

			fipc_test_blocking_recv_start( forw, &rxF );
			//temp = *rxF;
			fipc_recv_msg_end( forw, rxF );

			fipc_test_blocking_send_start( forw, &txF );
			//(*txF) = temp;
			fipc_send_msg_end( forw, txF );

			fipc_test_blocking_recv_start( back, &rxB );
			//temp = *rxB;	
			fipc_recv_msg_end( back, rxB );

			fipc_test_blocking_send_start( back, &txB );
			//(*txB) = temp;
			fipc_send_msg_end( back, txB );
		}

		fipc_test_blocking_recv_start( back, &rxB );
		message_t temp = *rxB;	
		fipc_recv_msg_end( back, rxB );

		fipc_test_blocking_send_start( back, &txB );
		(*txB) = temp;
		fipc_send_msg_end( back, txB );
	}


	// #ifndef FIPC_TEST_TIME_PER_TRANSACTION
	// 	start = RDTSC_START();
	// #else
	// 	register uint64_t* times = malloc( TRANSACTIONS * sizeof( uint64_t ) );
	// #endif

	// {
	// 	#ifdef FIPC_TEST_TIME_PER_TRANSACTION
	// 		start = RDTSC_START();
	// 	#endif

	// 	if ( rank == 0 )
	// 	{
	// 		fipc_test_blocking_send_start( chan, &tx );
	// 		tx->regs[0] = func1( 0 );
	// 		fipc_send_msg_end ( chan, tx );
	// 	}
	// 	else if ( rank == 1 )
	// 	{
	// 		fipc_test_blocking_recv_start( chan, &rx );
	// 		uint64_t temp = func2( rx->regs[0] );
	// 		fipc_recv_msg_end( chan, rx );

	// 		fipc_test_blocking_send_start( chan, &tx );
	// 		tx->regs[0] = temp;
	// 		fipc_send_msg_end( chan, tx );
	// 	}
	// 	else if ( rank == 2 )
	// 	{
	// 		fipc_test_blocking_recv_start( chan, &rx );
	// 		uint64_t temp = func3( rx->regs[0] );
	// 		fipc_recv_msg_end( chan, rx );

	// 		fipc_test_blocking_send_start( chan, &tx );
	// 		tx->regs[0] = temp;
	// 		fipc_send_msg_end( chan, tx );
	// 	}
	// 	else if ( rank == 3 )
	// 	{
	// 		fipc_test_blocking_recv_start( chan, &rx );
	// 		func4(rx->regs[0]);
	// 		fipc_recv_msg_end( chan, rx );
	// 	}

	// 	end = RDTSCP();

	// 	times[transaction_id] = end - start;
	// }

	// if ( rank == 0 )
	// {
	// 	fipc_test_stat_print_info( times, TRANSACTIONS );

	// 	fipc_test_blocking_send_start( chan, &tx );
	// 	fipc_send_msg_end ( chan, tx );
	// }
	// else if ( rank == 1 )
	// {
	// 	fipc_test_blocking_recv_start( chan, &rx );
	// 	fipc_recv_msg_end( chan, rx );

	// 	fipc_test_stat_print_info( times, TRANSACTIONS );

	// 	fipc_test_blocking_send_start( chan, &tx );
	// 	fipc_send_msg_end( chan, tx );
	// }
	// else if ( rank == 2 )
	// {
	// 	fipc_test_blocking_recv_start( chan, &rx );
	// 	fipc_recv_msg_end( chan, rx );

	// 	fipc_test_stat_print_info( times, TRANSACTIONS );

	// 	fipc_test_blocking_send_start( chan, &tx );
	// 	fipc_send_msg_end( chan, tx );
	// }
	// else if ( rank == 3 )
	// {
	// 	fipc_test_blocking_recv_start( chan, &rx );
	// 	fipc_recv_msg_end( chan, rx );

	// 	fipc_test_stat_print_info( times, TRANSACTIONS );
	// }


//	printf( "Hello from process %d.\n", rank );

	//if ( rank != 3 )
	//	fipc_test_shm_unlink( shm_keys[rank] );

	//fipc_test_shm_free_half_channel( chan );

	fipc_fini();
	return 0;
}
