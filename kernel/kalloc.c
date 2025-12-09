#include <stddef.h>
#include "defs.h"
#include "list.h"
#include "types.h"

/* Symbols provided by the linker script */
extern char HEAP_START[];
extern char HEAP_END[];

extern int kprintf(const char *s, ...);

#define ALIGNMENT 8U
#define MIN_PAYLOAD ALIGNMENT
#define MIN_BLOCKSIZE (sizeof(MemHeader_t) + MIN_PAYLOAD)

list_t alloc_list;
list_t free_list;

typedef struct __attribute__((aligned(ALIGNMENT))) MemHeader {
    uint32_t start_addr;
    size_t size;
    list_t list;
} MemHeader_t;

static inline uint32_t _align_up(uint32_t address)
{
    uint32_t mask = ALIGNMENT - 1;
    return (address + mask) & (~mask);
}

static inline uint32_t _align_down(uint32_t address)
{
    uint32_t mask = ALIGNMENT - 1;
    return address & (~mask);
}

static inline void memheader_init(MemHeader_t *hdr, size_t size)
{
    hdr->start_addr = (uint32_t)(hdr + 1);
    hdr->size = size;
    list_init(&hdr->list);
}

static inline uint32_t _block_end(MemHeader_t *hdr, size_t size)
{
    return hdr->start_addr + size;
}

void kmem_init()
{
    list_init(&alloc_list);
    list_init(&free_list);

    uint32_t heap_start = _align_up((uint32_t) HEAP_START);
    uint32_t heap_end = _align_down((uint32_t) HEAP_END);

    MemHeader_t *hdr = (MemHeader_t *) heap_start;
    uint32_t payload_start = (uint32_t)(hdr + 1);
    uint32_t payload_size = heap_end - payload_start;

    if (payload_start >= heap_end)
        panic("Heap is too small !");

    memheader_init(hdr, payload_size);
    list_insert_after(&free_list, &hdr->list);
    kprintf("MemHeader_t size : %d\n", sizeof(MemHeader_t));
    kprintf("heap_start : %x\n", heap_start);
    kprintf("heap_end : %x\n", heap_end);
}

static void *kmem_alloc(size_t size)
{
    size_t request = _align_up(size);

    for (list_t *node = free_list.next; node != &free_list; node = node->next) {
        MemHeader_t *hdr = list_entry(node, MemHeader_t, list);
        if (hdr->size < request)
            continue;

        uint32_t block_end = _block_end(hdr, hdr->size);
        MemHeader_t *new_hdr = (MemHeader_t *) _block_end(hdr, request);

        if ((uint32_t) new_hdr + MIN_BLOCKSIZE <= block_end) {
            hdr->size = request;

            uint32_t new_payload_addr = (uint32_t)(new_hdr + 1);
            size_t new_payload_size = block_end - new_payload_addr;
            memheader_init(new_hdr, new_payload_size);

            list_replace(&hdr->list, &new_hdr->list);
        } else
            list_remove(&hdr->list);
        list_insert_before(&alloc_list, &hdr->list);
        return (void *) hdr->start_addr;
    }
    return NULL;
}

static void kmem_coalesce(MemHeader_t *cur_hdr)
{
    if (cur_hdr->list.prev != &free_list) {
        MemHeader_t *prev_hdr =
            list_entry(cur_hdr->list.prev, MemHeader_t, list);

        uint32_t prev_end = _block_end(prev_hdr, prev_hdr->size);
        uint32_t cur_end = _block_end(cur_hdr, cur_hdr->size);

        if (prev_end + MIN_BLOCKSIZE > (uint32_t) cur_hdr) {
            prev_hdr->size += (cur_end - prev_end);
            list_remove(&cur_hdr->list);
            cur_hdr = prev_hdr;
        }
    }
    if (cur_hdr->list.next != &free_list) {
        MemHeader_t *next_hdr =
            list_entry(cur_hdr->list.next, MemHeader_t, list);

        uint32_t next_end = _block_end(next_hdr, next_hdr->size);
        uint32_t cur_end = _block_end(cur_hdr, cur_hdr->size);

        if (cur_end + MIN_BLOCKSIZE > (uint32_t) next_hdr) {
            cur_hdr->size += (next_end - cur_end);
            list_remove(&next_hdr->list);
        }
    }
}

static void kmem_free(void *p)
{
    if (!p)
        return;

    MemHeader_t *hdr = ((MemHeader_t *) p) - 1;

    list_remove(&hdr->list);

    for (list_t *node = free_list.next; node != &free_list; node = node->next) {
        MemHeader_t *cur_hdr = list_entry(node, MemHeader_t, list);

        if (cur_hdr->start_addr > hdr->start_addr) {
            list_insert_before(node, &hdr->list);
            break;
        }
    }
    kmem_coalesce(hdr);
}

void *kalloc(size_t size)
{
    if (size > 0)
        return kmem_alloc(size);
    else
        return NULL;
}

void kfree(void *p)
{
    kmem_free(p);
}

void kalloc_test()
{
    void *p1 = kalloc(18);
    kprintf("p1 : %p\n", p1);
    void *p2 = kalloc(40);
    kprintf("p2 = %p\n", p2);
    void *p3 = kalloc(62);
    kprintf("p3 = %p\n", p3);

    /* 測試 free 後的 block，可以被 kalloc 取用 */
    kfree(p2);
    void *p4 = kalloc(35);
    kprintf("p4 = %p\n", p4);

    /* 測試 coalesce 後的 block，可以被 kalloc 取用 */
    kfree(p4);
    kfree(p3);
    void *p5 = kalloc(50);
    kprintf("p5 = %p\n", p5);
    kfree(p1);
    kfree(p5);
}