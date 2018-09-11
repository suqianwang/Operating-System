/*
 File: my_allocator.c
 
 Author: <your name>
 Department of Computer Science
 Texas A&M University
 Date  : <date>
 
 Modified:
 
 This file contains the implementation of the module "MY_ALLOCATOR".
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "my_allocator.h"
#include "free_list.h"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

FL_HEADER * header;
char * _memory_start;

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FUNCTIONS FOR MODULE MY_ALLOCATOR */
/*--------------------------------------------------------------------------*/

/* Don't forget to implement "init_allocator" and "release_allocator"! */
unsigned int init_allocator(unsigned int _basic_block_size, unsigned int _length) {
    printf("\n***** executing init_allocator() *****\n");
    
    char * _memory_start = (char *) malloc(_length);
    header = FL_HEADER_init(_memory_start);
    header->length = _length;
    printf("malloc() allocated memory length: %d\npointer to the free list: %p\n", _length, header);
    int _remainder_length = _length - sizeof(*header);
    if (_remainder_length <= 0)
        return 0;
    printf("the amount of memory made available to the allocator: %d\n", _remainder_length);
    return _remainder_length;
}

int release_allocator() {
    free(_memory_start);
    return 0;
}

Addr my_malloc(size_t _length) {
    /* This preliminary implementation simply hands the call over the
     the C standard library!
     Of course this needs to be replaced by your implementation.
     */
    printf("\n\n***** executing my_malloc() *****\n");
    printf("allocating %zu bytes memory\n", _length);
    
    if (header == NULL) {
        printf("no memory to allocate\n");
        return NULL;
    }
    
    // find block with enouch memory in the free list, if none of them are enough, iterator will equal null.
    FL_HEADER* iterator = header;
    while (iterator != NULL) {
        if ( (iterator->length - sizeof(*header)) >= (int)_length ) {
            break;
        }
        printf("current memory block is not large enough, checking for the next memory block...\n");
        iterator = iterator->next;
    }
    
    if (iterator == NULL) {
        printf("no memory block is large enough to allocate\n");
        return NULL;
    }
    
    FL_HEADER* remove_block = iterator;
    header = FL_remove(header, remove_block);
    
    int remain_block_length = (int)(remove_block->length - sizeof(*header) - _length);
    printf("length of the remain block: %d\n", remain_block_length);
    FL_HEADER * allocate_block = remove_block;
    allocate_block->length = remove_block->length - remain_block_length;
    
    if (remain_block_length <= sizeof(*header)) {
        printf("\nallocate the whole removed block to user\n");
        return NULL;
    }
    else {
        FL_HEADER * remain_block = (FL_HEADER*)( (char*)remove_block + sizeof(*header) + _length);
        remain_block->length = remain_block_length;
        printf("starting address for the remain block: %p\n", remain_block);
        header = FL_add(header, remain_block);
        char* allocated_memory = (char*)allocate_block + sizeof(*header);
        printf("starting address for the allocated memory: %p\n", allocated_memory);
        return allocated_memory;
    }
}

int my_free(Addr _a) {
    /* Same here! */
    printf("\n\n***** executing my_free() *****\n");
    printf("free allocated memory, the address is %p\n", _a);
    if (_a == NULL) {
        printf("nothing to free\n");
        return 1;
    }
    
    FL_HEADER* return_block = (FL_HEADER*)((char*)_a - sizeof(*header));
    printf("length of the return block: %d\n", return_block->length);
    header = FL_add(header, return_block);
    return 0;
}

