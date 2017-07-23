#ifndef _PERF_COUNTER_HELPER_H
#define _PERF_COUNTER_HELPER_H

#include <asm/msr.h>    /* for r(w)dmsrl_safe */

typedef union {
    struct {
        u8 evt_sel;
        u8 umask;
        u8 usr_mode:1,
        os_mode:1,
        edge_detect:1,
        pin_ctrl:1,
        int_enable:1,
        any_thread:1,
        enable:1,
        invert_cmask:1;
        u8 ctr_mask;
        u32 reserved;
    };
    u64 reg;
} evt_sel_t;

typedef enum {
    OS_MODE = 1,
    USR_MODE = 2,
} ev_mode_t;

struct perf_cap {
    union {
        struct {
            u8 version;
            u8 num_gpc;
            u8 bit_width;
            u8 _len;
        };
        u32 reg;
    } eax;

    union {
        struct {
            u8 no_cc:1,
                no_ir:1,
                no_rc:1,
                no_cr:1,
                no_cm:1,
                no_bi:1,
                no_bm:1,
                reserved:1;
            u8 _dummy[3];
        };
        u32 reg;
    } ebx;
    
    union {
        struct {
            u8 ffcounters:5;
            u8 ctr_width;
            u32 reserved:19;
        };
        u32 reg;
    } edx;
};

/* Architectural performance counters - Addresses remain constant
 * across cpu families
 */
/* From arch/x86/include/asm/msr-index.h */
/* Intel Core-based CPU performance counters */
#define MSR_CORE_PERF_FIXED_CTR0	    0x00000309
#define MSR_CORE_PERF_FIXED_CTR1	    0x0000030a
#define MSR_CORE_PERF_FIXED_CTR2	    0x0000030b
#define MSR_CORE_PERF_FIXED_CTR_CTRL	0x0000038d
#define MSR_CORE_PERF_GLOBAL_STATUS	    0x0000038e
#define MSR_CORE_PERF_GLOBAL_CTRL	    0x0000038f
#define MSR_CORE_PERF_GLOBAL_OVF_CTRL	0x00000390

/* event select register of GP perf counter */
#define MSR_IA32s_EVNTSEL0		        0x00000186
#define MSR_IA32_PERFCTR0		        0x000000c1


static inline void create_event(u8 evt_sel, u8 umask, ev_mode_t mode,
					evt_sel_t *e)
{
    e->reg = 0ull;
    e->evt_sel = evt_sel;
    e->umask = umask;
    if (mode == OS_MODE)
        e->os_mode = 1;
    else
        e->usr_mode = 1;

    e->enable = 1;
}

static inline void write_evtsel(evt_sel_t *e, u64 gpidx)
{
        wrmsrl_safe(MSR_IA32s_EVNTSEL0 + gpidx, e->reg);
}

static inline void reset_evtsel(u64 gpidx)
{
        wrmsrl_safe(MSR_IA32s_EVNTSEL0 + gpidx, 0ull);
}

static inline void read_pmc(u64 *val, u64 gpidx)
{
	rdmsrl_safe(MSR_IA32_PERFCTR0 + gpidx, val);
}

static inline void reset_pmc(u64 gpidx)
{
	wrmsrl_safe(MSR_IA32_PERFCTR0 + gpidx, 0ull);
}



#endif /* _PERF_COUNTER_HELPER_H */
