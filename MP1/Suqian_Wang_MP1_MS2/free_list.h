//
//  free_list.h
//  MP1
//
//  Created by Susan Wang on 9/16/17.
//  Copyright © 2017 Suqian Wang. All rights reserved.
//

#ifndef free_list_h
#define free_list_h

typedef struct fl_header {
    int length;
    struct fl_header * next;
    struct fl_header * prev;
} FL_HEADER;

FL_HEADER* FL_HEADER_init(char * start);

FL_HEADER* FL_remove(FL_HEADER * free_list, FL_HEADER * block);
/* Remove the given block from given free list. The free-list
 pointer points to the first block in the free list. Depending
 on your implementation you may not need the free-list pointer.*/

FL_HEADER* FL_add(FL_HEADER * free_list, FL_HEADER * block);
/* Add a block to the free list. */

#endif /* free_list_h */
