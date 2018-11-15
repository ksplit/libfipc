/**
 * @File     : ticketlock.h
 * @Author   : Minjun Cha
 */

#include <stdio.h>
#include <stdint.h>

#ifndef LIBFIPC_TEST
#include "../libfipc_test.h"
#endif

typedef unsigned int uint;

struct thread_ticketlock {
  volatile unsigned int current_ticket;
  volatile unsigned int next_ticket;       
};

void thread_ticket_spin_init(struct thread_ticketlock *lk);

void thread_ticket_spin_lock(struct thread_ticketlock *lk);

void thread_ticket_spin_unlock(struct thread_ticketlock *lk);

