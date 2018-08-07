#include "fipc_numa.h"

int match_cpus(uint32_t** producer_cpus, uint32_t** consumer_cpus)
{
    	struct bitmask *cm;	
        int num_nodes;
        int n;
        unsigned long cpu_bmap;
        int numa_present;
        int ret = 0;
        int possible_cpus = numa_num_configured_cpus();

        struct numa_config *config;
        struct node *nodes;

        struct task_placement *all_cpus;

        // numa_available returns 0 if numa apis are available, else -1
        if ((ret = numa_present = numa_available())) {
                printf("Numa apis unavailable!\n");
                goto err_numa;
        }

        config = calloc(1, sizeof(struct numa_config));

        if (!config) {
                perror("calloc:");
                goto err_numa;
        }

        config->num_nodes = num_nodes = numa_num_configured_nodes();
        cm = numa_allocate_cpumask();

        config->nodes = nodes = calloc(num_nodes, sizeof(struct node));

        *producer_cpus = calloc(possible_cpus, sizeof(uint32_t));
        *consumer_cpus = calloc(possible_cpus, sizeof(uint32_t));

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

        int prod_id = 0;
        int cons_id = 0;
    
        for (n = 0; n < num_nodes; n++) {
                int cpu;
		int num_cpus = nodes[n].num_cpus;

//                printf("Node: %d cpu_bitmask: 0x%08lx | num_cpus: %d\n", n, nodes[n].cpu_bitmask,
//                                               nodes[n].num_cpus);
                
                for (cpu = 0; cpu < num_cpus / 2; cpu++) {
			int next = num_nodes * (num_cpus / 2);
			
			(*producer_cpus)[prod_id] = nodes[n].cpu_list[cpu];
		        (*producer_cpus)[prod_id + next] = nodes[n].cpu_list[cpu + (num_cpus / 2)];
			(*consumer_cpus)[cons_id] = (*producer_cpus)[prod_id + next];
			(*consumer_cpus)[cons_id + next] = (*producer_cpus)[prod_id];
			
			++prod_id;
			++cons_id;
                }
//              printf("\n");
                free(nodes[n].cpu_list);
        }
/*
	int i;
	for (i = 0; i < 64; ++i){
	        printf("producer_cpus[%d] :  %d, consumer_cpus[%d] : %d\n", i, (*producer_cpus)[i], i, (*consumer_cpus)[i]); 
	}
*/
        
        free(nodes);
	return 0;
err_range:
        numa_free_cpumask(cm);
err_numa:
	return ret;
}

