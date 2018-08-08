
#include "../libfipc_test.h"

#define _GNU_SOURC
#define RESET_MASK(x)           ~(1LL << (x))

// CPUS Placement Policy
#define PROD_CONS_SEPARATE_NODES        1
#define PROD_CONS_SAME_NODES            2
#define PROD_CONS_MIXED_MODE            3

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
/*
enum numa_policy {
        PROD_CONS_SEPARATE_NODES = 1,
        PROD_CONS_SAME_NODES = 2,
        PROD_CONS_MIXED_MODE = 3,
        NUM_POLICIES,
};

struct task_placement {
        uint32_t *producer_cpus;
        uint32_t *consumer_cpus;
}policies[NUM_POLICIES];
*/
int match_cpus(uint32_t** p, uint32_t** c, int policy);

