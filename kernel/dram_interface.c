#include <linux/dram_interface.h>
#include <linux/module.h>

#define NUM_HEAP_IDENTS
static traverse_func_ptr traverse_funcs[NUM_HEAP_IDENTS];

void register_traverse(int identifier, traverse_func_ptr traverse) {
	traverse_funcs[identifier] = traverse;
}
EXPORT_SYMBOL(register_traverse);

