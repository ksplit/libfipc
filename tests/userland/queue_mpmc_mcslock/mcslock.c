/**
 * @File     : mcslock.c
 * @Author   : Jiwon Jeon
 */

#include "mcslock.h"

void mcs_init_global ( mcslock* L )
{
    L = NULL;
	//pr_err("mcs init\n");
}

void mcs_init_local	( qnode* I )
{
	I->next = NULL;
	I->waiting = 0;
}

void mcs_lock ( mcslock* L, qnode* I )
{
	I->next = NULL;

	mcslock *pred = fetch_and_store(&L, &I);

	if ( pred == NULL)
		return;
	I->waiting = 1;
	pred->next = I;

	while ( I->waiting != 0)
		;
}

void mcs_unlock ( mcslock* L, qnode* I )
{
	mcslock *succ;
	if ( !(succ = I->next) )
	{
		if ( cmp_and_swap( L, (uint64_t)I, (uint64_t)NULL) )
			return;
		do
		{
			succ = I->next;
		} while ( !succ );
	}
	succ->waiting = 0;
}

