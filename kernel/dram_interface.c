#include <linux/dram_interface.h>
#include <linux/module.h>
#include <linux/mm_types.h>
#include <linux/sched.h>
#include <asm/current.h>
#include <linux/slab.h>

#define NUM_HEAP_IDENTS 10

static traverse_func_ptr traverse_funcs[NUM_HEAP_IDENTS];

void register_traverse(int identifier, traverse_func_ptr traverse) {
	traverse_funcs[identifier] = traverse;
}
EXPORT_SYMBOL(register_traverse);

struct free_chunk_info* call_traverse(void* arena_ptr, size_t page_no, size_t* len) {
	struct mm_struct* cur_mm;
	int ident;

	cur_mm = current->mm;

	down_read(&cur_mm->heap_info_lock);

	if (!cur_mm->heap_info) {
		printk(KERN_ERR "No Registered Heap!");
		return;
	}
	
	ident = cur_mm->heap_info->identifier;

	up_read(&cur_mm->heap_info_lock);

	

	struct free_chunk_info* ret;

	ret = traverse_funcs[ident](arena_ptr, page_no, len);

	return ret;
}


// pending testing
asmlinkage long sys_register_heap_info (int mem_allocator_identifier, 
										void* arena_start_ptr, 
										void* subheap_start_ptr, 
										size_t subheap_size, 
										int* new_error_info_flag) {

	struct mm_struct* cur_mm;
	struct heap_info* new_info;

	cur_mm = current->mm;
	
	

	new_info = kmalloc(sizeof(struct heap_info), GFP_KERNEL);
	new_info->identifier = mem_allocator_identifier;
	new_info->arena_ptr = arena_start_ptr;
	new_info->heapseg_start_ptr = subheap_start_ptr;
	new_info->size = subheap_size;
	new_info->new_error_info_flag = new_error_info_flag;
	new_info->next = NULL;


	

	struct heap_info* last_info;


	down_write(&cur_mm->heap_info_lock);

	last_info = cur_mm->heap_info;

	while (last_info && last_info->next) {
		last_info = last_info->next;
	}

	if (!last_info) {
		cur_mm->heap_info = new_info;
	} else {
		last_info->next = new_info;
	}

	up_write(&cur_mm->heap_info_lock);

	return 0;

}


// solely for testing purposes
asmlinkage long sys_test_traverse (void* arena_start_ptr,  size_t VpageNO, size_t* len) {
	struct mm_struct* cur_mm;
	int ident;

	cur_mm = current->mm;

	down_read(&cur_mm->heap_info_lock);

	if (!cur_mm->heap_info) {
		printk(KERN_WARNING "No Registered Heap!");
	}
	
	ident = cur_mm->heap_info->identifier;

	up_read(&cur_mm->heap_info_lock);

	struct free_chunk_info* ret =  traverse_funcs[ident](arena_start_ptr, VpageNO, len);

	// do something here yourself

	return 0;

}

// returns 0 on success, -1 on failure
// deletes the subheap when arena_size == 0
asmlinkage long sys_update_heap_info (int mem_allocator_identifier, void* subheap_start_ptr, size_t arena_size) {
	struct mm_struct* cur_mm;
	cur_mm = current->mm;

	struct heap_info* last_info;


	down_write(&cur_mm->heap_info_lock);

	last_info = cur_mm->heap_info;

	while (last_info) {
		if (last_info->heapseg_start_ptr == subheap_start_ptr) {
			last_info->size = arena_size;

			if (arena_size == 0) {
				// delete
				if (last_info->next) {
					last_info->next = last_info->next->next;
				}
			}
			return 0;
		}

        
		last_info = last_info->next;
	}

	return -1;

	up_write(&cur_mm->heap_info_lock);
} 

asmlinkage long sys_get_error_info (void** buf, size_t arena_start_addr, size_t count) {
	
}

asmlinkage long sys_reserve_header (void* vaddr, size_t len) {
	struct mm_struct* cur_mm;
	struct header_info* new_info;

	cur_mm = current->mm;
	
	

	new_info = kmalloc(sizeof(struct header_info), GFP_KERNEL);
	new_info->header_ptr = vaddr;
	new_info->header_size = len;
	new_info->next = NULL;


	struct header_info* last_info;


	down_write(&cur_mm->heap_info_lock);

	last_info = cur_mm->header_info;

	while (last_info && last_info->next) {
		last_info = last_info->next;
	}

	if (!last_info) {
		cur_mm->header_info = new_info;
	} else {
		last_info->next = new_info;
	}

	up_write(&cur_mm->heap_info_lock);

	return 0;
}  


asmlinkage long sys_dump_heaps (void) {
	struct mm_struct* cur_mm;
	cur_mm = current->mm;

	struct heap_info* last_info;


	down_read(&cur_mm->heap_info_lock);

	last_info = cur_mm->heap_info;

	printk(KERN_INFO "Start dumping heaps for current process");

	while (last_info) {
		printk(KERN_INFO "arena_ptr 0x%llx subheap_start_ptr 0x%llx size %llu",
			last_info->arena_ptr, last_info->heapseg_start_ptr, last_info->size);

	}

	printk(KERN_INFO "Finished dumping heaps");

	return 0;

	up_read(&cur_mm->heap_info_lock);
}