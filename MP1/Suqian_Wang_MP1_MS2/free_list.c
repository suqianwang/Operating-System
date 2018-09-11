//
//  free_list.c
//  MP1
//
//  Created by Susan Wang on 9/16/17.
//  Copyright © 2017 Suqian Wang. All rights reserved.
//

#include "free_list.h"
#include <stdio.h>

FL_HEADER* FL_HEADER_init(char * start) {
    printf("---initial free list header---");
    FL_HEADER* header = (FL_HEADER *) start;
    header->length = 0;
    header->next = NULL;
    header->prev = NULL;
    printf("\n header length: %d\n next pointer: %p\n previous pointer: %p\n", header->length, header->next, header->prev);
    return header;
}

FL_HEADER* FL_remove(FL_HEADER * free_list, FL_HEADER * block) {
    /* Remove the given block from given free list. The free-list
     pointer points to the first block in the free list. Depending
     on your implementation you may not need the free-list pointer.*/
    if (block->next == NULL && block->prev == NULL) {
        free_list = NULL;
    }
    else if (block->next == NULL) {
        block->prev->next = NULL;
        block->prev = NULL;
    }
    else if (block == free_list) {
        free_list = block->next;
        free_list->prev = NULL;
        block->next = NULL;
    }
    else {
        block->next->prev = block->prev;
        block->prev->next = block->next;
        block->prev = NULL;
        block->next = NULL;
    }
    return free_list;
}

FL_HEADER* FL_add(FL_HEADER * free_list, FL_HEADER * block) {
    /* Add a block to the free list. */
    if (free_list == NULL) {
        free_list = block;
        free_list->next = NULL;
        free_list->prev = NULL;
        free_list->length = block->length;
    }
    else {
        free_list->prev = block;
        block->next = free_list;
        free_list = block;
    }
    return free_list;
}
