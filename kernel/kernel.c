extern int kprintf(const char *, ...);
extern int kscanf(const char *, ...);
extern void kecho(void);
extern void uart_init(void);
extern void kmem_init(void);
extern void sched_init(void);
extern void schedule(void);
extern void trap_init(void);
extern void timer_init(void);

/* test */
extern void kalloc_test(void);
extern void loadTasks(void);
extern void timer_test(void);

void test_io()
{
    int x, y;
    char z[128];
    kprintf("Input x, y: ");
    kscanf("%d %d", &x, &y);
    kprintf("x * y = %d\n", x * y);
    kscanf("%s", z);
    kprintf("%s\n", z);
}

void start_kernel(void)
{
    uart_init();
    kmem_init();
    trap_init();
    timer_init();
    sched_init();
    timer_test();
    kprintf("\033[2J\033[H");
    kprintf("Hello, RVOS!\n");
    schedule();
    while (1)
        ;
}