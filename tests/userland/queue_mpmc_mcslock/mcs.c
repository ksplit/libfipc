/**
 * @File     : mcs.c
 */

#include "mcs.h"

void mcs_init_global ( mcslock** L )
{
	*L = NULL;
	//pr_err("mcs init\n");
}

void mcs_init_local	( qnode* I )
{
	I->next = NULL;
	I->waiting = 0;
}

void mcs_lock ( mcslock** L, qnode* I )
{
	volatile qnode* pred = fetch_and_store(L, I);

	if ( pred == NULL ) {
printf("\n[lock] empty\n");
		return;
    }
	I->waiting = 1;
	pred->next = I;

	while ( I->waiting != 0)
    {    
        //printf("spin_lock\n");
    }
printf("\n[lock] not\n");
	__sync_synchronize();
}

void mcs_unlock ( mcslock** L, qnode* I )
{
    __sync_synchronize();
	if ( !(I->next) )
	{
		if ( cmp_and_swap( L, (uint64_t)I, (uint64_t)NULL ) ) {
printf("[unlock] last\n\n");
			return;
        }
		while ( !I->next )
		{
//printf("spin_unlock\n");
		}
	}
	I->next->waiting = 0;
printf("[unlock] not\n\n");
}

