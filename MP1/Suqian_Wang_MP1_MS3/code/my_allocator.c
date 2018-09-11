/*
 File: my_allocator.c
 
 Author: Suqian Wang
 Department of Computer Science
 Texas A&M University
 Date  : Sep 25, 2017
 
 Modified:
 
 This file contains the implementation of the module "MY_ALLOCATOR".
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

#define FL_HEADER_PTR_SIZE (sizeof(FL_HEADER*))
#define FL_HEADER_SIZE (sizeof(FL_HEADER))

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "my_allocator.h"
#include "free_list.h"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

// starting address of the memory pool
char * _memory_start;
// pointer to free list header
FL_HEADER ** fl_ptr_header;
// basic block size and maximum block size after initialization
unsigned int min_block_size;
unsigned int max_block_size;
// number of free list (including empty list)
unsigned int fl_number;

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

// aux-functions
unsigned int max_block(unsigned int remain_length);
unsigned int index_to_block_size(int index);
unsigned int block_size_to_index(int block_size);
//void print(FL_HEADER** fl_ptr_header);
FL_HEADER* split_block(FL_HEADER* block);
FL_HEADER* find_buddy(FL_HEADER* block);
FL_HEADER* merge_block(FL_HEADER* block);

/*--------------------------------------------------------------------------*/
/* FUNCTIONS FOR MODULE MY_ALLOCATOR */
/*--------------------------------------------------------------------------*/

unsigned int init_allocator(unsigned int _basic_block_size, unsigned int _length) {
    printf("\n***** executing init_allocator() *****\n");
    min_block_size = _basic_block_size;
    max_block_size = max_block(_length);
    fl_number = log(max_block_size / min_block_size)/log(2.0) + 1;
    fl_ptr_header = (FL_HEADER**)malloc(fl_number * FL_HEADER_PTR_SIZE);
    _memory_start = (char *)malloc(_length);
    printf("initial memory pool starts at: %p\n", _memory_start);
    
    // traverse the free list header array and initialize each free list, if it is empty, initialize its header to NULL; if not, initialize a block with its corresponding block size
    unsigned int block_size = max_block_size;
    unsigned int remain_length = _length - block_size;
    char* fl_header = _memory_start;
    for (int i = 0; i < fl_number; i++) {
        if (index_to_block_size(i) != block_size) {
            fl_ptr_header[i] = NULL;
        }
        else {
            fl_ptr_header[i] = FL_HEADER_init(fl_header, block_size);
            fl_header = fl_header + block_size;
            block_size = max_block(remain_length);
            remain_length = remain_length - block_size;
        }
    }
    
    // print the free lists after initialization
//    print(fl_ptr_header);
//    return (_length - remain_length);
}

int release_allocator() {
    // free allocated memory
    free(fl_ptr_header);
    free(_memory_start);
    return 0;
}

Addr my_malloc(size_t _length) {
    printf("\n\n***** executing my_malloc() *****\n");
    printf("user allocating %zu bytes memory\n", _length);
    // find the smallest block for user
    int user_block_size = pow(2, ceil(log(_length + FL_HEADER_SIZE)/log(2.0)));
    printf("need to find a block at least %d bytes\n", user_block_size);
    
    // loop through free list header array and find the current smallest block in the free list that is enough for user, if it is two times bigger than user required, split the block recursively.
    FL_HEADER* current_min_block = fl_ptr_header[fl_number-1];
    for (int i = fl_number-1; i >= 0; i--) {
        if(fl_ptr_header[i] != NULL) {
            current_min_block = fl_ptr_header[i];
            if (user_block_size <= current_min_block->length) {
                if((current_min_block->length < 2 * user_block_size) || (current_min_block->length == min_block_size)) {
                    fl_ptr_header[i] = FL_remove(fl_ptr_header[i], current_min_block);
                    
                    // print the free lists after allocation
//                    printf("\nAfter my_malloc, the list become: \n");
//                    print(fl_ptr_header);
                    return ((char*)current_min_block + FL_HEADER_SIZE);
                }
                while ((current_min_block->length >= 2 * user_block_size) && (current_min_block->length > min_block_size)) {
                    fl_ptr_header[i] = FL_remove(fl_ptr_header[i], current_min_block);
                    FL_HEADER* new_block = split_block(current_min_block);
                    i++;
                    fl_ptr_header[i] = FL_add(fl_ptr_header[i], new_block);
                    fl_ptr_header[i] = FL_add(fl_ptr_header[i], current_min_block);
                    // print the free lists after allocation
//                    print(fl_ptr_header);
//                    current_min_block = fl_ptr_header[i];
                }
                fl_ptr_header[i] = FL_remove(fl_ptr_header[i], current_min_block);
                
                // print the free lists after allocation
//                printf("\nAfter my_malloc, the list become: \n");
//                print(fl_ptr_header);
                return ((char*)current_min_block + FL_HEADER_SIZE);
            }
        }
    }
    printf("ERROR: not enough memory to allocate\n");
    return NULL;
}

int my_free(Addr _a) {
    printf("\n\n***** executing my_free() *****\n");
    if (_a == NULL) {
        printf("ERROR: invalid address\n");
        return 1;
    }
    
    // check the length of merged block, merge block recursively until the length of the merged block is the same as before
    FL_HEADER* block_to_merge = (FL_HEADER*)((char*)_a - FL_HEADER_SIZE);
    int index = block_size_to_index(block_to_merge->length);
    fl_ptr_header[index] = FL_add(fl_ptr_header[index], block_to_merge);
    while (index != 0) {
        block_to_merge = merge_block(block_to_merge);
        if (block_size_to_index(block_to_merge->length) == index) {
            break;
        }
        index = block_size_to_index(block_to_merge->length);
    }
    
    // print the free lists after free
//    printf("\nAfter my_free, the list become: \n");
//    print(fl_ptr_header);
    return 0;
}

/*--------------------------------------------------------------------------*/
/* HELPER FUNCTIONS FOR MODULE MY_ALLOCATOR */
/*--------------------------------------------------------------------------*/

// find the maximum block whose size is of a power of 2 from the remain memory pool
unsigned int max_block(unsigned int remain_length) {
    return pow(2, floor(log(remain_length)/log(2.0)));
}

// find the corresponding block size of a free list given the index of the free list among all free lists
unsigned int index_to_block_size(int index) {
    if (index >= fl_number) {
        printf("invalid index");
        return 0;
    }
    return (max_block_size/pow(2, index));
}

// find the corresponding index of the free list among all free lists given the block size of a free list
unsigned int block_size_to_index(int block_size) {
    if ((block_size > max_block_size) || (block_size < min_block_size)) {
        printf("invalid block size\n");
        return 0;
    }
    return log(max_block_size/block_size)/log(2.0);
}

// print all free lists
//void print(FL_HEADER** fl_ptr_header) {
//    for (int i = 0; i < fl_number; i++) {
//        if (fl_ptr_header[i] != NULL) {
//            printf("< %d > : ", fl_ptr_header[i]->length);
//            FL_print(fl_ptr_header[i]);
//        }
//        else {
//            printf("< %d > : \n", index_to_block_size(i));
//        }
//    }
//}

// split a block in the half
FL_HEADER* split_block(FL_HEADER* block) {
    FL_HEADER* new_block = (FL_HEADER*)((char*)block + block->length/2);
    new_block->length = block->length/2;
    block->length = block->length/2;
    return new_block;
}

// find the buddy of a given address
FL_HEADER* find_buddy(FL_HEADER* block) {
    return (FL_HEADER*)((((char*)block - _memory_start)^(block->length)) + _memory_start);
}

// merge a block with its buddy if the buddy exist, and add the merged block to its corresponing free list. If can't find the buddy, just add the block to its corresponding free list
FL_HEADER* merge_block(FL_HEADER* block) {
    int index = block_size_to_index(block->length);
    FL_HEADER* buddy = find_buddy(block);
    FL_HEADER* iterator = fl_ptr_header[index];
    
    // loop through the corresponding list of the block and find its buddy, if the buddy exist, add the merged block to the list whose block size is twice
    while (iterator != NULL) {
        if (iterator == buddy) {
            fl_ptr_header[index] = FL_remove(fl_ptr_header[index], buddy);
            fl_ptr_header[index] = FL_remove(fl_ptr_header[index], block);
            if (block < buddy) {
                fl_ptr_header[index-1] = FL_add(fl_ptr_header[index-1], block);
                block->length = 2 * block->length;
                return block;
            }
            else {
                fl_ptr_header[index-1] = FL_add(fl_ptr_header[index-1], buddy);
                buddy->length = 2 * buddy->length;
                return buddy;
            }
        }
        else {
            iterator = iterator->next;
        }
    }
    printf("\ncan't find its buddy in the list.\n");
    return block;
}

