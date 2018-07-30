/**
 * @File     : mcslock.c
 * @Author   : Jiwon Jeon
 */
#include "mcslock.h"

void mcs_init ( mcslock *L )
{
    L->v = NULL;
    for (int i = 0; i < MAX_MCS_LOCKS; i++)
        if ( !lock_used[i].v )
        {
            if (cmp_and_swap_atomic( &lock_used[i], 0, 1) )
            {
                L->lock_idx = i;
                return;
            }
        }
    pr_err("mcs init: Oops");
}

void mcs_lock ( mcslock *L )
{
    volatile struct qnode *mynode = &I[L->lock_idx];
    mynode->next = NULL;
    struct qnode *predecessor = (struct qnode *)fetch_and_store(L, (uint64_t)mynode);
    if (predecessor) {
        mynode->locked = 1;
        predecessor->next = mynode;
        while (mynode->locked)
        {
            //nop_pause();
        }
    }
}

void mcs_unlock ( mcslock *L )
{
    volatile struct qnode *mynode = &I[L->lock_idx];
    if (!mynode->next) {
        if ( cmp_and_swap(L, (uint64_t)mynode, 0) ) {
            return;
        }
        while ( !mynode->next )
        {
            //nop_pause();
        }
    }
    ((struct qnode *)mynode->next)->locked = 0;
}


