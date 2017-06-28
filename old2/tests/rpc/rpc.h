#include <linux/kernel.h>

#include "../../IPC/ipc.h"

typedef enum {NULL_INVOCATION, ADD_CONSTANT, ADD_NUMS, ADD_3_NUMS,
	      ADD_4_NUMS, ADD_5_NUMS, ADD_6_NUMS} fn_types;

/* must be divisible by 6... because I call 6 functions in the Callee.c */
#define TRANSACTIONS 60


void callee(struct ttd_ring_channel *chan);
void caller(struct ttd_ring_channel *chan);
