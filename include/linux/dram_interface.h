#ifndef DRAM_INTERFACE_H
#define DRAM_INTERFACE_H

typedef free_chunk_info* (*traverse_func_ptr) (void*, size_t, size_t*);

struct free_chunk_info {
 		void* start; // pointer to the free chunk
   	size_t len; // size of the free chunk
};

void register_traverse(int identifier, traverse_func_ptr traverse); 

#endif
