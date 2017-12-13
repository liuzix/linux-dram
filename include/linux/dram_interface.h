#ifndef DRAM_INTERFACE_H
#define DRAM_INTERFACE_H

#include <linux/types.h>

struct free_chunk_info {
    void* start; // pointer to the free chunk
    size_t len; // size of the free chunk
};

typedef struct free_chunk_info* (*traverse_func_ptr) (void*, size_t, size_t*);

void register_traverse(int identifier, traverse_func_ptr traverse); 
struct free_chunk_info* call_traverse(void* arena_ptr, size_t page_no, size_t* len) ;


#endif
