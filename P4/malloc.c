#include "malloc.h"
#include <stdio.h>
#include <assert.h>

//Magni Mar Magnason - magni21
// Þórir Hrafn Harðarson - thorirhh21

typedef struct _Block {
    
    // Pointer to the header of the next free block.
    // Only valid if this block is also free.
    // This is null for the last Block of the free list

    struct _Block *next;

    // Our header should always have a size of 16 Bytes.
    // This is just for 32 bit systems

    uint8_t padding[8 - sizeof(void*)];

    // The size of this block, including the header
    // Always a multiple of 16 bytes

    uint64_t size;

    // The data area of this block.

    uint8_t data[];
} Block;

#define HEADER_SIZE sizeof(Block)
#define INV_HEADER_SIZE_MASK ~((uint64_t)HEADER_SIZE - 1)

// This is the heap you should use.
// 16 MiB heap space per default. The heap does not grow.

#define HEAP_SIZE (HEADER_SIZE * 1024 * 1024)
uint8_t __attribute__ ((aligned(HEADER_SIZE))) _heapData[HEAP_SIZE];

// This should point to the first free block in memory.

Block *_firstFreeBlock;

// Initializes the memory block. You don't need to change this.

void initAllocator()
{
    _firstFreeBlock = (Block*)&_heapData[0];
    _firstFreeBlock->next = NULL;
    _firstFreeBlock->size = HEAP_SIZE;
}

// Gets the next block that should start after the current one.

static Block *_getNextBlockBySize(const Block *current)
{
    static const Block *end = (Block*)&_heapData[HEAP_SIZE];
    Block *next = (Block*)&current->data[current->size - HEADER_SIZE];
    assert(next <= end);
    return (next == end) ? NULL : next;
}

// Dumps the allocator. You should not need to modify this.

void dumpAllocator()
{
    Block *current;

    printf("All blocks:\n");
    current = (Block*)&_heapData[0];
    while (current) {
        //---------------Debug printf-------------
        //printf("current size : %lld\n", current->size);
        if (current->next != NULL)
        //    printf("current size next : %lld\n", current->next->size);
        //----------------------------------------
        assert((current->size & INV_HEADER_SIZE_MASK) == current->size);
        assert(current->size > 0);

        printf("  Block starting at %" PRIuPTR ", size %" PRIu64 "\n",
            ((uintptr_t)(void*)current - (uintptr_t)(void*)&_heapData[0]),
            current->size);

        current = _getNextBlockBySize(current);
    }

    current = _firstFreeBlock;
    while (current) {
        printf("  Free block starting at %" PRIuPTR ", size %" PRIu64 "\n",
            ((uintptr_t)(void*)current - (uintptr_t)(void*)&_heapData[0]),
            current->size);

        current = current->next;
    }
}

// Round the integer up to the block header size (16 Bytes).

uint64_t roundUp(uint64_t n)
{
    (void) n;
    int remainder = 0;
    //TODO: Implement
    if (n > 0)
    {
        remainder = n%16;
        if (remainder == 0)
        {
            return n;
        }
        else
        {
            n = n + (16 - remainder);
            return n;
        }
    }
    else
        return 0;
}


void *my_malloc(uint64_t size)
{
    (void) size;
    // TODO: Implement
    //Make sure data is being passed to malloc
    if (size <= 0) { return NULL; }
    //Round up size of data to nearest 16 and add header size
    size = roundUp(size) + HEADER_SIZE;
    //int heapIndex = 0;

    Block *current;
    Block *previous;
    Block *dataPtr;
    //set the current block to the first free block
    current = _firstFreeBlock;
    previous = current;
    //Loop through at least once
    if (current == NULL)
    {
        return NULL;
    }
    do
    {
        previous = current;
        //printf("Current size free: %lld \n", current->size);
        //If the block has more space than is needed then split the block
        if (current->size > size)  
        {
            //Make a new block of free memory that points to the heap 
            //where the size of the current block ends
            uint64_t sizeRemaining = current->size - size;
            current->size=size;
            Block *newFreeBlock;
            newFreeBlock = _getNextBlockBySize(current);
            newFreeBlock->size = sizeRemaining;
            newFreeBlock->next = current->next;
            current->next = NULL;
            _firstFreeBlock = newFreeBlock;
            //Set the size of the current block that is being filled and 
            //set its pointer to NULL since it is no longer free
            current->size=size;
            current->next=NULL;
            //newFreeBlock = _getNextBlockBySize(current);
            //_firstFreeBlock = newFreeBlock;
            //return a pointer to the allocated data 
            dataPtr = (Block*)&current->data[0];
            return dataPtr;
        }
        if (current->size == size)
        {
            dataPtr = (Block*)&current->data[0];
            if (current->next != NULL)
            {
                _firstFreeBlock = current->next;
            }
            else
            {
                _firstFreeBlock = NULL;
            }
            return dataPtr;
        } 
        if (current->size < size)
        {
            //Current doesn't have enough free space but remember it as
            //the first free block of space
            _firstFreeBlock = previous;
            
            if (current->next == NULL)
            {
                return NULL;
            }
        }   
        //Set current to next pointer in the list if there is one
        if (current->next != NULL) 
        {
            current = current->next; 
        }
        
        if (current->next == NULL) 
        {
            return NULL;
        }

    }
    while (current->next != NULL);
    //if no free block was found in the list return NULL
    return NULL; 
}

void my_free(void *address)
{
    (void) address;

    // TODO: Implement
    if (address == NULL)
    {
        return;
    }

    address = address - HEADER_SIZE;

    Block* newFreeBlock = (Block*)address;
    
    if (address < (void*)_firstFreeBlock) // insert at beginning
    {
        _firstFreeBlock->size = newFreeBlock->size+_firstFreeBlock->size;
        _firstFreeBlock = address;
        return;
    }

    Block* current = (Block*)_firstFreeBlock;
    
    while (1) 
    {

        if (address > (void*)current)
        {
            if (address < (void*)current->next) 
            {
                newFreeBlock->next = current->next;
                current->next = newFreeBlock;
                return;
            }
        }

        if (current->next==NULL)
        {
            current->next = newFreeBlock;
            newFreeBlock->next = NULL;
            return;
        }

    }
}