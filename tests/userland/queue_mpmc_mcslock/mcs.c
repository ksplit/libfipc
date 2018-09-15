/**
 * @File	: mcs.c
 */

#include "mcs.h"

void mcs_init_global( mcslock** L)
{
	*L = NULL;
}

void mcs_lock ( mcslock **L, qnode* I)
{
        qnode *tail;

        I->next = NULL;
        I->spin = 0;

        tail = xchg_64(L, I);

        /* No one there? */
        if (!tail) return;

        /* Someone there, need to link in */
        tail->next = I;

        /* Make sure we do the above setting of next. */
         barrier();

       /* Spin on my spin variable */
        while (!I->spin) cpu_relax();

        return;
}

void mcs_unlock ( mcslock** L, qnode* I)
{
        /* No successor yet? */
        if (!I->next)
        {
            /* Try to atomically unlock */
            if (cmpxchg(L, I, NULL) == I) return;

            /* Wait for successor to appear */
            while (!I->next) cpu_relax();
        }

        /* Unlock next one */
        I->next->spin = 1;
}
