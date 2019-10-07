#ifndef _NUMA_HELPER_H_
#define _NUMA_HELPER_H_

#include <numa.h>
#include <stdio.h>
#include <stdint.h>

#define RESET_MASK(x)		~(1LL << (x))

struct numa_node {
	unsigned long cpu_bitmask;
	unsigned int num_cpus;
	uint32_t *cpu_list;
};

struct numa_node *get_numa_config(void);

#endif /* _NUMA_HELPER_H_ */
