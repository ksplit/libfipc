/**
 * @File     : test.h
 * @Author   : Abdullah Younis
 *
 * This test measures the latency of a long message going through a pipeline
 * of N processors.
 *
 * NOTE: This test assumes an x86 architecture.
 *
 * NOTE: This test assumes a computer with 2-32 processing units.
 */

#include "../libfipc_test.h"

//#define FIPC_TEST_TIME_PER_TRANSACTION

#define CHANNEL_ORDER    ilog2(sizeof(message_t)) + 16
#define TRANSACTIONS     100000LU
#define NUM_PROCESSORS   4
#define MSG_LENGTH       2

const uint64_t cpu_map[] =
{
	1,
	5,
	9,
	13,
	17,
	21,
	25,
	29,
	0,
	4,
	8,
	12,
	16,
	20,
	24,
	28,
	2,
	6,
	10,
	14,
	18,
	22,
	26,
	30,
	3,
	7,
	11,
	15,
	19,
	23,
	27,
	31
};

const char* shm_keys[] =
{
	"FIPC_LM_S0",
	"FIPC_LM_S1",
	"FIPC_LM_S2",
	"FIPC_LM_S3",
	"FIPC_LM_S4",
	"FIPC_LM_S5",
	"FIPC_LM_S6",
	"FIPC_LM_S7",
	"FIPC_LM_S8",
	"FIPC_LM_S9",
	"FIPC_LM_S10",
	"FIPC_LM_S11",
	"FIPC_LM_S12",
	"FIPC_LM_S13",
	"FIPC_LM_S14",
	"FIPC_LM_S15",
	"FIPC_LM_S16",
	"FIPC_LM_S17",
	"FIPC_LM_S18",
	"FIPC_LM_S19",
	"FIPC_LM_S20",
	"FIPC_LM_S21",
	"FIPC_LM_S22",
	"FIPC_LM_S23",
	"FIPC_LM_S24",
	"FIPC_LM_S25",
	"FIPC_LM_S26",
	"FIPC_LM_S27",
	"FIPC_LM_S28",
	"FIPC_LM_S29",
	"FIPC_LM_S30",
	"FIPC_LM_S31"
};
