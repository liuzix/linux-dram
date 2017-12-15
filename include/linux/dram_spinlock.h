#ifndef _SPINLOCK_CMPXCHG_H
#define _SPINLOCK_CMPXCHG_H

#include <linux/sched.h>
#include <asm/current.h>

struct {
    volatile unsigned int lock;
    volatile pid_t thread_id;
} dram_spinlock;

#define SPINLOCK_ATTR __attribute__((always_inline, no_instrument_function))

/* Pause instruction to prevent excess processor bus usage */
#define cpu_relax() asm volatile("pause\n": : :"memory")

SPINLOCK_ATTR void dram_spin_lock(struct dram_spinlock *lock)
{
    //long tid = syscall(__NR_gettid);
    long tid = current->pid;
    if (tid == lock->thread_id) { 
        // if the lock is already held by the current thread
        return;
    }

    tid = (tid << 32) + 1;
    while (!__sync_bool_compare_and_swap((u64*)lock, 0, tid)) {
        cpu_relax();
    }
}

SPINLOCK_ATTR void dram_spin_unlock(struct dram_spinlock *s)
{
    s->thread_id = 0;
    s->lock = 0;
}

#define SPINLOCK_INITIALIZER { 0 }
#endif