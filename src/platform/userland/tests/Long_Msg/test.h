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
#define TRANSACTIONS     1000000LU
#define NUM_PROCESSORS   32
#define MSG_LENGTH       5

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
	"FIPC_NFV_S0_F",
	"FIPC_NFV_S1_F",
	"FIPC_NFV_S2_F",
	"FIPC_NFV_S3_F",
	"FIPC_NFV_S4_F",
	"FIPC_NFV_S5_F",
	"FIPC_NFV_S6_F",
	"FIPC_NFV_S7_F",
	"FIPC_NFV_S8_F",
	"FIPC_NFV_S9_F",
	"FIPC_NFV_S10_F",
	"FIPC_NFV_S11_F",
	"FIPC_NFV_S12_F",
	"FIPC_NFV_S13_F",
	"FIPC_NFV_S14_F",
	"FIPC_NFV_S15_F",
	"FIPC_NFV_S16_F",
	"FIPC_NFV_S17_F",
	"FIPC_NFV_S18_F",
	"FIPC_NFV_S19_F",
	"FIPC_NFV_S20_F",
	"FIPC_NFV_S21_F",
	"FIPC_NFV_S22_F",
	"FIPC_NFV_S23_F",
	"FIPC_NFV_S24_F",
	"FIPC_NFV_S25_F",
	"FIPC_NFV_S26_F",
	"FIPC_NFV_S27_F",
	"FIPC_NFV_S28_F",
	"FIPC_NFV_S29_F",
	"FIPC_NFV_S30_F",
	"FIPC_NFV_S31_F"
};
