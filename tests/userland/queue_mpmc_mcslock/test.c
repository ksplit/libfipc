#include <stdio.h>
#include <stdlib.h>

typedef struct qnode {
  volatile int waiting;
  volatile void* next;
} qnode;

static inline int
//cas ( qnode *a, int cmp, int new )
cas ( int *a, int cmp, int new )
{
  int out;
  __asm__ volatile(
    "lock; cmpxchg %2, %1"
    : "=a" (out), "+m" (*a)
    : "q" (new), "0" (cmp)
    : "cc");
  return out==cmp;

}
//static inline qnode* 
static inline int*
//fas( qnode *a, qnode *b )
fas( int **a, int *b )
{ 
  int* ret;
  __asm__ volatile(
        "lock; xchgq %2, %1 \n\t"
        : "=a" (*a), "=r" (ret)
        : "m" (*a), "0" (b)
        : "memory", "cc");
  return ret;
}

int main() {

  qnode* q1, *q2,*store;
  int n1, n2, *n3, n4, n5;
  n1 = 1;
  n2 = 1;
  n4 = 3;
  n5 = 0;
  int *p1, *p2;
  p1 = &n1;
  p2 = &n2;
  q1= (qnode*) malloc(sizeof(qnode));
  q1->waiting = 10;
  q2= (qnode*) malloc(sizeof(qnode));
  q2->waiting = 20;


// fas test
printf("[before] store: %p\t q1: %p\t q2: %p\t\n", n3, p1, p2);
//printf("[befor2] store: %p\t q1: %x\t q2: %x\t\n", n3, *p1, *p2);
    n3 = fas(&p1, p2);  
printf("[after ] store: %p\t q1: %p\t q2: %p\t\n", n3, p1, p2);
//printf("[after2] store: %p\t q1: %x\t q2: %x\t\n", n3, *p1, *p2);

/* 
// cas test
printf("[before] n1: %d\t n2: %d\t n4: %d\t n5: %d\n", n1, n2, n4, n5);
    n5 = cas(&n1, n2, n4);  
printf("[after ] n1: %d\t n2: %d\t n4: %d\t n5: %d\n", n1, n2, n4, n5);
*/
}
