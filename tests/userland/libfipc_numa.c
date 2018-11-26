#include "libfipc_numa.h"


int match_cpus(uint32_t** producer_cpus, uint32_t** consumer_cpus, int policy, int producer_count, int consumer_count)
{
    struct bitmask *cm;
    int num_nodes;
    int n, i, j;
    unsigned long cpu_bmap;
    int numa_present;
    int ret = 0;
    int prod_id = 0;
    int cons_id = 0;
    int possible_cpus = numa_num_configured_cpus();
    int prod_cpus[possible_cpus];
    int cons_cpus[possible_cpus];

    struct numa_config *config;
    struct node *nodes;

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

    for (n = 0; n < num_nodes; n++)
    {
        int num_cpus, cpus = 0;
        if ((ret = numa_node_to_cpus(n, cm)))
        {
            fprintf(stderr, "bitmask is not long enough\n");
            goto err_range;
        }

        nodes[n].cpu_bitmask = cpu_bmap = *(cm->maskp);
        nodes[n].num_cpus = num_cpus = __builtin_popcountl(cpu_bmap);
        nodes[n].cpu_list = calloc(sizeof(uint32_t), num_cpus);

        // extract all the cpus from the bitmask
        while (cpu_bmap)
        {
            // cpu number starts from 0, ffs starts from 1.
            unsigned long c = __builtin_ffsll(cpu_bmap) - 1;
            cpu_bmap &= RESET_MASK(c);
            nodes[n].cpu_list[cpus++] = c;
        }
    }
/*
 if nodes state like below
    0  1

    3  2
*/
    int node_order[2][4] = {{0,1,2,3},{0,3,1,2}};
    int order_type = policy == DIFF_NODE ? 1 : 0;
    int num_cpus = nodes[0].num_cpus;
  
    for (i = 0; i < num_nodes; i++)
    {
        int cpu;
	int n = node_order[order_type][i]; 
	int half = policy == DIFF_NODE ? num_cpus : num_cpus / 2;
	int machine = num_nodes == 4 ? D820 : C6420;

        for ( cpu = 0; cpu < half; cpu++ )
	{
	    if ( policy == DIFF_NODE ) 
	    {
		int next_node = machine == D820? node_order[1][i+2] : 1;

		(*producer_cpus)[prod_id++] = nodes[n].cpu_list[cpu];
                (*consumer_cpus)[cons_id++] = nodes[next_node].cpu_list[cpu];
	
		if( i == (num_nodes/2 - 1)  && cpu == half - 1 ) i = num_nodes; 
	    }
            else if( policy == SAME_NODE_NON_SIBLING ) // only hyper on
 	    {
	        int temp = cpu >= num_cpus/4 ? num_cpus/4 : 0 ;
		int hop = machine == D820 ? 4 : 8 ;
		
		(*producer_cpus)[prod_id++] = nodes[n].cpu_list[cpu + temp];
                (*consumer_cpus)[cons_id++] = nodes[n].cpu_list[cpu + temp + hop];
	    }
	    else if ( policy == SAME_NODE_SIBLING )
	    {
		(*producer_cpus)[prod_id++] = nodes[n].cpu_list[cpu];
                (*consumer_cpus)[cons_id++] = nodes[n].cpu_list[cpu + half];
	    }
	}
    }
/*
    prod_id = 0; cons_id = 0;
    for(i = num_nodes-1; i >= 0; i--)
    {
	if( producer_count*2 > i * num_cpus )
	{
printf("%d<<<", i);
	    for( j = 0; j < num_cpus/2 ; j++)
	    { 
    	        for( n = 0; n < i; n++)
	        {
		    (*producer_cpus)[prod_id++] = prod_cpus[ j + n*num_cpus/2 ];
		    (*consumer_cpus)[cons_id++] = cons_cpus[ j + n*num_cpus/2 ];   
    	        }
	    }
	    break;
	}
    }
*/
/*
    for (n = 0; n < num_nodes; n++)
    {
        int cpu;
        int num_cpus = nodes[n].num_cpus;

//        printf("Node: %d cpu_bitmask: 0x%08lx | num_cpus: %d\n", n, nodes[n].cpu_bitmask,
//                                               nodes[n].num_cpus);

        for ( cpu = 0; cpu < num_cpus / 2; cpu++ )
        {
            int next = num_nodes * (num_cpus / 2);
            int bias = 0;

            if( policy == PROD_CONS_SEPARATE_NODES )
            {
                (*producer_cpus)[prod_id] = nodes[n].cpu_list[cpu];
                (*producer_cpus)[prod_id + next] = nodes[n].cpu_list[cpu + (num_cpus / 2)];

                (*consumer_cpus)[cons_id] = nodes[(num_nodes-1)-n].cpu_list[cpu + (num_cpus / 2)];
                (*consumer_cpus)[cons_id + next] = nodes[(num_nodes-1)-n].cpu_list[cpu];

            }
            else if( policy == PROD_CONS_SAME_NODES )
            {
                (*producer_cpus)[prod_id] = nodes[n].cpu_list[cpu];
                (*producer_cpus)[prod_id + next] = nodes[n].cpu_list[cpu + (num_cpus / 2)];

                (*consumer_cpus)[cons_id] = nodes[n].cpu_list[cpu + (num_cpus / 2)];
                (*consumer_cpus)[cons_id + next] = nodes[n].cpu_list[cpu];

            }	   
	    else if( policy == PROD_CONS_MIXED_MODE )
            {
                int temp = n;
                int index = 0;

                if( n % 2 == 1){
                    temp = n-1;
                    index = num_cpus / 4; 
                } 
                if( cons_id % 2 == 1)
                	bias = 1;

                (*producer_cpus)[prod_id] = nodes[n].cpu_list[cpu];
                (*producer_cpus)[prod_id + next] = nodes[n].cpu_list[cpu + (num_cpus / 2)];

                (*consumer_cpus)[cons_id] = nodes[temp+bias].cpu_list[cpu/2 + index +  (num_cpus / 2)];
                (*consumer_cpus)[cons_id + next] = nodes[temp+bias].cpu_list[cpu/2 + index];

            }
	    else
	    {
		int temp = 0;
		if( cpu >= 4 ){
		    temp = 4;
		}
		(*producer_cpus)[prod_id] = nodes[n].cpu_list[cpu + temp];
	
   		(*consumer_cpus)[cons_id] = nodes[n].cpu_list[cpu + temp + (num_cpus/4)];
	    }
            ++prod_id;
            ++cons_id;
        }
    }*/
    
    free(config);

    for (n = 0; n < num_nodes; n++)
        free(nodes[n].cpu_list);

    free(nodes);
    return 0;
err_range:
        numa_free_cpumask(cm);
err_numa:
        return ret;
}
