#ifndef _SPINLOCK_CMPXCHG_H
#define _SPINLOCK_CMPXCHG_H

#include <linux/sched.h>
#include <asm/current.h>

struct {
    volatile unsigned int lock;
    volatile pid_t thread_id;
} dram_spinlock;

#define SPINLOCK_ATTR static __inline __attribute__((always_inline, no_instrument_function))

/* Pause instruction to prevent excess processor bus usage */
#define cpu_relax() asm volatile("pause\n": : :"memory")

SPINLOCK_ATTR char __testandset(struct dram_spinlock *p, long old_val, long new_val)
{
    char result = 0;
    asm volatile (
            "lock; cmpxchgq %4, %1; sete %0"
            : "=q" (result), "=m" (*p)
            : "m" (*p), "a" (old_val), "r" (new_val)
            : "memory");
    return (!result);
}

SPINLOCK_ATTR void dram_spin_lock(struct dram_spinlock *lock)
{
    //long tid = syscall(__NR_gettid);
    long tid = current->pid;
    if (tid == lock->thread_id) { 
        // if the lock is already held by the current thread
        return;
    }

    tid = (tid << 32) + 1;
    while (__testandset(lock, 0, tid)) {
        cpu_relax();
    }
}

SPINLOCK_ATTR void dram_spin_unlock(struct dram_spinlock *s)
{
    long new_val = 0;
    asm volatile ("xchgq %0,%1"
                :
                :"m" (*s), "r" (new_val)
                :"memory");
}

#define SPINLOCK_INITIALIZER { 0 }
#endif