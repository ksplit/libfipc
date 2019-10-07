#include "numa_helper.h"

struct numa_node *get_numa_config(void)
{
	struct numa_node *nodes = NULL;
	struct bitmask *cm;
	int num_nodes;
	int n;
	unsigned long cpu_bmap;
	int numa_present;
	int ret = 0;

	// numa_available returns 0 if numa apis are available, else -1
	if ((ret = numa_present = numa_available())) {
		printf("Numa apis unavailable!\n");
		goto err_numa;
	}

	printf("numa_available: %s\n", numa_present ? "false" : "true");

	printf("numa_max_possible_node: %d\n", numa_max_possible_node());
	printf("numa_num_possible_nodes: %d\n", numa_num_possible_nodes());
	printf("numa_max_node: %d\n", numa_max_node());

	num_nodes = numa_num_configured_nodes();
	cm = numa_allocate_cpumask();

	nodes = calloc(sizeof(struct numa_node), num_nodes);

	printf("numa_num_configured_nodes: %d\n", numa_num_configured_nodes());
	printf("numa_num_configured_cpus: %d\n", numa_num_configured_cpus());
	printf("numa_num_possible_cpus: %d\n", numa_num_possible_cpus());

	for (n = 0; n < num_nodes; n++) {
		int num_cpus, cpus = 0;
		if ((ret = numa_node_to_cpus(n, cm))) {
			fprintf(stderr, "bitmask is not long enough\n");
			goto err_range;
		}

		nodes[n].cpu_bitmask = cpu_bmap = *(cm->maskp);
		nodes[n].num_cpus = num_cpus = __builtin_popcountl(cpu_bmap);
		nodes[n].cpu_list = calloc(sizeof(uint32_t), num_cpus);

		// extract all the cpus from the bitmask
		while (cpu_bmap) {
			// cpu number starts from 0, ffs starts from 1.
			unsigned long c = __builtin_ffsll(cpu_bmap) - 1;
			cpu_bmap &= RESET_MASK(c);
			nodes[n].cpu_list[cpus++] = c;
		}
	}

	for (n = 0; n < num_nodes; n++) {
		int cpu;
		printf("Node: %d cpu_bitmask: 0x%08lx | num_cpus: %d\n", n, nodes[n].cpu_bitmask,
						nodes[n].num_cpus);
		for (cpu = 0; cpu < nodes[n].num_cpus; cpu++) {
		       printf("%d ", nodes[n].cpu_list[cpu]);
		}
		printf("\n");
	}

err_range:
	numa_free_cpumask(cm);
err_numa:
	return nodes;
}
