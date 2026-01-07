extern int kprintf(const char *, ...);
extern int kscanf(const char *, ...);
extern void kecho(void);
extern void uart_init(void);
extern void kalloc_test(void);
extern void kmem_init(void);
extern void loadTasks(void);
extern void sched_init(void);
extern void schedule(void);

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
    sched_init();
    loadTasks();
    kprintf("Hello, RVOS!\n");

    // kalloc_test();

    schedule();
    while (1)
        ;
}