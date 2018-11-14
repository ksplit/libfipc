#include "libfipc_test.h"

#define _GNU_SOURC
#define RESET_MASK(x)           ~(1LL << (x))

// CPUS Placement Policy
#define DIFF_NODE        	1
#define SAME_NODE_NON_SIBLING   2
#define SAME_NODE_SIBLING       3

#define D820  0
#define C6420 1

struct node {
        unsigned long cpu_bitmask;
        unsigned int num_cpus;
        uint32_t *cpu_list;
};

struct numa_config {
        int num_nodes;
        int total_cpus;
        struct node *nodes;
};

int match_cpus(uint32_t** p, uint32_t** c, int policy);

