#include <linux/hashtable.h>
#include <linux/slab.h>
#include <linux/fault_table.h>

/* Defines a hashtable to store all the faulty cells information */
DEFINE_HASHTABLE(fault_list, 16);

/* We use a slab allocator to mitigate the allocation cost of hashtable nodes */
struct kmem_cache* fault_list_node_pool = NULL;

void __init fault_table_init(void) {
    fault_list_node_pool = kmem_cache_create("fault_list",
                                             sizeof(fault_node_t), 
                                             8, 
                                             SLAB_POISON | SLAB_RED_ZONE, 
                                             NULL);
}

void fault_table_insert(u64 addr) {
    fault_node_t* new_node = kmem_cache_alloc(fault_list_node_pool, GFP_KERNEL);
    new_node->address = addr;
    u64 hashkey = addr & (~0xFFF);
    hash_add(fault_list, &new_node->node, hashkey);
    printk(KERN_INFO "Inserted faulty address 0x%llx", addr);

} 
EXPORT_SYMBOL(fault_table_insert);
