/**
 * @File     : mcslock.h
 * @Author   : Jiwon Jeon
 */

#define MAX_MCS_LOCKS        2

// Types
typedef uint64_t data_t;

typedef struct qnode {
    volatile void* CACHE_ALIGNED next; 
    volatile char CACHE_ALIGNED locked; 
} qnode; 

typedef struct {
    struct qnode* CACHE_ALIGNED v;
    int CACHE_ALIGNED lock_idx;
} mcslock;

typedef struct node {
	uint64_t CACHE_ALIGNED field;	
} node_t;

volatile struct qnode I[MAX_MCS_LOCKS];
int lock_used[MAX_MCS_LOCKS];

void mcs_init(mcslock *L);
void mcs_lock(mcslock *L);
void mcs_unlock(mcslock *L);




