#ifndef MALLOC_H
#define MALLOC_H
#include <inttypes.h>

void initAllocator();
void dumpAllocator();
uint64_t roundUp(uint64_t n);
void *my_malloc(uint64_t size);
void my_free(void *address);

#endif

