/**
 * @File     : ticketlock.c
 * @Author   : Minjun Cha
 */

#include "ticketlock.h"

void
thread_ticket_spin_init(struct thread_ticketlock *lk)
{
  lk->current_ticket = lk->next_ticket = 0;
}

void
thread_ticket_spin_lock(struct thread_ticketlock *lk)
{
  int myticket = __sync_fetch_and_add(&lk->next_ticket, 1);
  while(myticket != lk->current_ticket)
    fipc_test_pause();

  __sync_synchronize();
}

void
thread_ticket_spin_unlock(struct thread_ticketlock *lk)
{
  __sync_synchronize();
  lk->current_ticket++;
}

