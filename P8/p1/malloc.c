#include "malloc.h"

#include <stdio.h>
#include <assert.h>
#include <pthread.h>

typedef struct _Block {
    /*
     * Pointer to the header of the next free block.
     * Only valid if this block is also free.
     * This is null for the last Block of the free list.
     */
    struct _Block *next;

    /*
     * Our header should always have a size of 16 Bytes.
     * This is just for 32 bit systems.
     */
    uint8_t padding[8 - sizeof(void*)];

    /*
     * The size of this block, including the header
     * Always a multiple of 16 bytes.
     */
    uint64_t size;

    /*
     * The data area of this block.
     */
    uint8_t data[];
} Block;

#define HEADER_SIZE sizeof(Block)
#define INV_HEADER_SIZE_MASK ~((uint64_t)HEADER_SIZE - 1)
#define ALLOCATED_BLOCK_MAGIC (Block*)(0xbaadf00d)


pthread_mutex_t lock;

/*
 * This is the heap you should use.
 * 16 MiB heap space per default. The heap does not grow.
 */
#define HEAP_SIZE (HEADER_SIZE * 1024 * 1024)
uint8_t __attribute__ ((aligned(HEADER_SIZE))) _heapData[HEAP_SIZE];

/*
 * This should point to the first free block in memory.
 */
Block *_firstFreeBlock;

/*
 * Initialize the memory block. You don't need to change this.
 */
void initAllocator()
{
    _firstFreeBlock = (Block*)&_heapData[0];
    _firstFreeBlock->next = NULL;
    _firstFreeBlock->size = HEAP_SIZE;

//    pthread_mutex_t lock;
    pthread_mutex_init(&lock,NULL);


}

/*
 * Get the next block that should start after the current one.
 */
static Block *_getNextBlockBySize(const Block *current)
{
    static const Block *end = (Block*)&_heapData[HEAP_SIZE];
    Block *next = (Block*)&current->data[current->size - HEADER_SIZE];

    assert(next <= end);
    return (next == end) ? NULL : next;
}

/*
 * Dumps the allocator. You should not need to modify this.
 */
void dumpAllocator()
{
    pthread_mutex_trylock (&lock); //lock

    Block *current;

    printf("All blocks:\n");
    current = (Block*)&_heapData[0];
    while (current) 
    {
//        pthread_mutex_lock (&lock); //lock
        assert((current->size & INV_HEADER_SIZE_MASK) == current->size);
        assert(current->size > 0);

        printf("  Block starting at %" PRIuPTR ", size %" PRIu64 " (%s)\n",
            ((uintptr_t)(void*)current - (uintptr_t)(void*)&_heapData[0]),
            current->size,
            (current->next == ALLOCATED_BLOCK_MAGIC) ? "allocated" : "free");

        current = _getNextBlockBySize(current);
//        pthread_mutex_unlock(&lock); //unlock
    }
    pthread_mutex_unlock(&lock); //unlock

    pthread_mutex_lock (&lock); //lock
    printf("Current free block list:\n");
    current = _firstFreeBlock;
    while (current) {
//        pthread_mutex_lock (&lock); //lock
        assert(current->next != ALLOCATED_BLOCK_MAGIC);

        printf("  Free block starting at %" PRIuPTR ", size %" PRIu64 "\n",
            ((uintptr_t)(void*)current - (uintptr_t)(void*)&_heapData[0]),
            current->size);

        current = current->next;
    //    pthread_mutex_unlock(&lock); //unlock
    }
    pthread_mutex_unlock(&lock); //unlock


   // pthread_mutex_unlock(&lock); //unlock

}

/*
 * Round the integer up to the block header size (16 Bytes).
 */
uint64_t roundUp(uint64_t n)
{
    return (n + HEADER_SIZE - 1) & INV_HEADER_SIZE_MASK;
}

static void *_allocate(Block **blockLink, uint64_t size)
{

    assert(blockLink != NULL);
    assert((size & INV_HEADER_SIZE_MASK) == size);
    assert(size >= HEADER_SIZE);

    Block *freeBlock = *blockLink;
    assert(freeBlock != NULL);
    assert(freeBlock->size >= size);

    if (freeBlock->size == size) {
        // The selected free block exactly fits the requested size. We thus
        // do not need to split the block.
        *blockLink = freeBlock->next;
    } else {
        // The free block is larger. We thus split it into two blocks. One
        // allocated that we return to the caller and a new free one.
        const uint64_t remainingSize = freeBlock->size - size;
        assert((remainingSize & INV_HEADER_SIZE_MASK) == remainingSize);
        assert(remainingSize >= HEADER_SIZE);

        freeBlock->size = size;

        Block *newFreeBlock = _getNextBlockBySize(freeBlock);
        newFreeBlock->size  = remainingSize;
        newFreeBlock->next  = freeBlock->next;
        *blockLink = newFreeBlock;
    }

    // Mark the current block as allocated by setting a magic next value
    freeBlock->next = ALLOCATED_BLOCK_MAGIC;
    
    pthread_mutex_unlock(&lock); //unlock

    return &freeBlock->data[0];
}

void *my_malloc(uint64_t size)
{
    // Calculate the minimum size of the free block we need to find.
    // We only allocate blocks that are multiples of 16 bytes in size, so we
    // round up the requested size. This potentially wastes some memory but
    // makes management easier. We also need to store our block header.
    pthread_mutex_lock (&lock); //lock
    const uint64_t requestedSize = roundUp(size) + HEADER_SIZE;
    Block *current = _firstFreeBlock;
    Block **link   = &_firstFreeBlock;

    while (current) {
        if (current->size >= requestedSize) {
            return _allocate(link, requestedSize);
        }

        // We did not find a large-enough block, yet. Move on to the next one.
        link    = &current->next;
        current = current->next;
    }

    pthread_mutex_unlock(&lock); //unlock
    // We did not find a free block that offers enough space. Return NULL to
    // indicate that the allocation failed.
    return NULL;
}

/*
 * Check if we can merge this block with the next one.
 */
static void _tryMerge(Block *freeBlock)
{
    assert(freeBlock != NULL);
    assert(freeBlock->next != ALLOCATED_BLOCK_MAGIC);

    // If there is no free block after the given one, there
    // is nothing we can merge.
    if (freeBlock->next == NULL) {
        return;
    }

    // Look at the next block in the heap. If it is linked with the free block
    // it is also free and we can merge the blocks.
    const Block *next = _getNextBlockBySize(freeBlock);

    if (next == freeBlock->next) {
        assert(next->next != ALLOCATED_BLOCK_MAGIC);

        freeBlock->size += next->size;
        freeBlock->next = next->next;
    }
}

void my_free(void *address)
{
    pthread_mutex_lock (&lock); //lock
    if (address == NULL) {
        return;
    }

    assert(address >= (void*)&_heapData[0]);
    assert(address <= (void*)&_heapData[HEAP_SIZE]);

    // We first need to get the block header for the given address.
    // address should point to the beginning of the block's data segment,
    // directly after the header -> decrement address by one header size.
    Block *block = (Block*)(address) - 1;
    assert(block->next == ALLOCATED_BLOCK_MAGIC);

    Block *freeBlock = _firstFreeBlock;

    // The free block must be inserted at the correct position in the free list
    // to allow merging with the neighbor blocks in the case they are free, too
    // We can find the correct position easily, by comparing the block addresses
    if ((freeBlock == NULL) || (freeBlock > block)) {
        // Insert the block at the front of the freelist
        _firstFreeBlock = block;
        block->next = freeBlock;

        _tryMerge(block);
    } else {
        // Iterate the free list until we reach its end or find the correct pos
        while ((freeBlock->next != NULL) && (freeBlock->next < block)) {
            freeBlock = freeBlock->next;
        }

        // Insert block into free list and try to merge with neighbor blocks
        block->next = freeBlock->next;
        freeBlock->next = block;

        _tryMerge(block);
        _tryMerge(freeBlock);
    }
    pthread_mutex_unlock(&lock); //unlock
}



//====== TEST 3: You do not deadlock in dumpAllocator, if called while your my_alloc lock is held =====
//Testing malloc.c with test3.c
//Starting to test malloc.c...
//All blocks:
//  Block starting at 0, size 64 (allocated)
//  Block starting at 64, size 16777152 (free)
//Current free block list:
//  Free block starting at 64, size 16777152
///home/sty22/A8p1/.tests/test3.c line 42: You don't use any locks in dumpAllocator
//You have errors in your solution, please fix them.
