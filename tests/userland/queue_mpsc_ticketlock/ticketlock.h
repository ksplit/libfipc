/**
 * @File     : ticketlock.h
 * @Author   : Minjun Cha
 */

#include <stdio.h>
#include <inttypes.h>

typedef unsigned int uint;

struct thread_ticketlock {
  unsigned int current_ticket;
  unsigned int next_ticket;       
};

void thread_ticket_spin_init(struct thread_ticketlock *lk);

void thread_ticket_spin_lock(struct thread_ticketlock *lk);

void thread_ticket_spin_unlock(struct thread_ticketlock *lk);

