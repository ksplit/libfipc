/**
 * @File     : test.h
 * @Author   : Abdullah Younis
 *
 * This test passes a simulated packet through a series of processes, which
 * represent composed functions. This is done using separate address spaces
 * to isolate the functions.
 *
 * NOTE: This test assumes an x86 architecture.
 *
 * NOTE: This test assumes a computer with 2-32 processing units.
 */

#include "../libfipc_test.h"

//#define FIPC_TEST_TIME_PER_TRANSACTION

#define CHANNEL_ORDER    ilog2(sizeof(message_t)) + 7
#define TRANSACTIONS     1000LU
#define NUM_PROCESSORS   4
#define LINES_PER_PACKET 1
#define MAX_LINES_USED   1

#include "functions.h"

typedef struct packet
{
	cache_line_t data[LINES_PER_PACKET] CACHE_ALIGNED;

} packet_t;

void* (* const pipe_func[])(void*, const uint64_t) =
{
	TTL_Decrement,
	TTL_Decrement,
	TTL_Decrement,
	TCPIP_checksum,
	TTL_Decrement,
	TTL_Decrement,
	TTL_Decrement,
	TCPIP_checksum,
	TTL_Decrement,
	TTL_Decrement,
	TTL_Decrement,
	TCPIP_checksum,
	TTL_Decrement,
	TTL_Decrement,
	TTL_Decrement,
	TCPIP_checksum,
	TTL_Decrement,
	TTL_Decrement,
	TTL_Decrement,
	TCPIP_checksum,
	TTL_Decrement,
	TTL_Decrement,
	TTL_Decrement,
	TCPIP_checksum,
	TTL_Decrement,
	TTL_Decrement,
	TTL_Decrement,
	TCPIP_checksum,
	TTL_Decrement,
	TTL_Decrement,
	TTL_Decrement,
	TCPIP_checksum
};

const uint64_t cpu_map[] =
{
	0,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	8,
	9,
	10,
	11,
	12,
	13,
	14,
	15,
	16,
	17,
	18,
	19,
	20,
	21,
	22,
	23,
	24,
	25,
	26,
	27,
	28,
	29,
	30,
	31
};

const char* shm_keys[] =
{
	"FIPC_NFV_S0",
	"FIPC_NFV_S1",
	"FIPC_NFV_S2",
	"FIPC_NFV_S3",
	"FIPC_NFV_S4",
	"FIPC_NFV_S5",
	"FIPC_NFV_S6",
	"FIPC_NFV_S7",
	"FIPC_NFV_S8",
	"FIPC_NFV_S9",
	"FIPC_NFV_S10",
	"FIPC_NFV_S11",
	"FIPC_NFV_S12",
	"FIPC_NFV_S13",
	"FIPC_NFV_S14",
	"FIPC_NFV_S15",
	"FIPC_NFV_S16",
	"FIPC_NFV_S17",
	"FIPC_NFV_S18",
	"FIPC_NFV_S19",
	"FIPC_NFV_S20",
	"FIPC_NFV_S21",
	"FIPC_NFV_S22",
	"FIPC_NFV_S23",
	"FIPC_NFV_S24",
	"FIPC_NFV_S25",
	"FIPC_NFV_S26",
	"FIPC_NFV_S27",
	"FIPC_NFV_S28",
	"FIPC_NFV_S29",
	"FIPC_NFV_S30",
	"FIPC_NFV_S31"
};
