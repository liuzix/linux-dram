#ifndef H_FAULT_TABLE
#define H_FAULT_TABLE

#include <linux/types.h>
#include <linux/hashtable.h>
#include <linux/mm_types.h>

void __init fault_table_init(void);

void fault_table_insert(u64 addr);

int fault_table_lookup_page(u64 addr, u32* results, size_t len);

void fault_page_cache_push(struct page* page) ;

/* if a page contains faults, returns 1. Otherwise 0 */
static inline int fault_table_has_fault(u64 addr) {
    return fault_table_lookup_page(addr, NULL, 0) > 0;
}

typedef struct {
    u64 address;
    struct hlist_node node;
} fault_node_t;

typedef struct {
    struct page* page;
    struct list_head node;
} fault_page_cache_node_t;

#endif