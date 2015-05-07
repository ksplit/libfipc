#include <linux/kernel.h>

#include "ipc.h"

typedef enum {ADD_CONSTANT, ADD_NUMS, ADD_3_NUMS,
	      ADD_4_NUMS, ADD_5_NUMS, ADD_6_NUMS} fn_types;
#define TRANSACTIONS 10002


void callee(struct ttd_ring_channel *chan);
void caller(struct ttd_ring_channel *chan);
