#ifndef H_FAULT_TABLE
#define H_FAULT_TABLE

#include <linux/types.h>
#include <linux/hashtable.h>

void __init fault_table_init(void);

void fault_table_insert(u64 addr);

int fault_table_lookup_page(u64 addr, u32* results, size_t len);

typedef struct {
    u64 address;
    struct hlist_node node;
} fault_node_t;

#endif