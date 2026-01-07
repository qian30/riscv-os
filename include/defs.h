#ifndef __DEFS_H__
#define __DEFS_H__

#include <stddef.h>
#include "types.h"

/* printf */
int kprintf(const char *s, ...);
void panic(char *s);

/* scanf */
int kscanf(const char *fmt, ...);

/* kalloc.c */
void *kalloc(size_t size);
void kfree(void *p);

/* task.c */
void schedule(void);
task_t *task_init(const char *, taskFunc_t, void *, size_t, uint16_t);
void task_startup(task_t *);
uint32_t task_resume(task_t *);
uint32_t task_yield(void);

/* spinlock.c */
void spinlock_init(spinlock_t *);
int acquire(spinlock_t *);
int release(spinlock_t *);

/* mem.c */
void *memset(void *src, int value, size_t size);
void *memcpy(void *dest, const void *src, size_t size);

#endif  // __DEFS_H__