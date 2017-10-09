#include <linux/hashtable.h>
#include <linux/slab.h>
#include <linux/fault_table.h>
#include <linux/rwlock.h>

/* Defines a hashtable to store all the faulty cells information */
DEFINE_HASHTABLE(fault_list, 16);

/* We use a slab allocator to mitigate the allocation cost of hashtable nodes */
struct kmem_cache* fault_list_node_pool = NULL;

/* We need to lock to protect the hash table */
DEFINE_RWLOCK(fault_list_lock);

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

    write_lock(&fault_list_lock);

    hash_add(fault_list, &new_node->node, hashkey);

    write_unlock(&fault_list_lock);

    printk(KERN_INFO "Inserted faulty address 0x%llx", addr);

} 
EXPORT_SYMBOL(fault_table_insert);

int fault_table_lookup_page(u64 addr, u32* results, size_t len) {
    read_lock(&fault_list_lock);

    fault_node_t *e;
    u64 hashkey = addr & (~0xFFF);

    u32* it = results;
    u32* it_end = results + len;
    hash_for_each_possible(fault_list, e, node, hashkey) {
        if ((e->address & (~0xFFF)) == hashkey) {
            if (it == it_end) {
                read_unlock(&fault_list_lock);
                return;
            }
            *it = (u32)e->address & 0xFFF; // calculate offset within page
            it++;
        }
    }

    read_unlock(&fault_list_lock);
}
EXPORT_SYMBOL(fault_table_lookup_page);