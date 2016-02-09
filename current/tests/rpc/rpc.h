#include <linux/kernel.h>

#include "../../IPC/ipc.h"

typedef enum {NULL_INVOCATION, ADD_CONSTANT, ADD_NUMS, ADD_3_NUMS,
	      ADD_4_NUMS, ADD_5_NUMS, ADD_6_NUMS} fn_types;

/* must be divisible by 6... because I call 6 functions in the Callee.c */
#define TRANSACTIONS 6


int callee(void *chan);
int caller(void *chan);
