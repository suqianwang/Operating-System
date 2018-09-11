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

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

char *_memory_start;
char *_remainder_start;
unsigned int _memory_length;
size_t _remainder_length;

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FUNCTIONS FOR MODULE MY_ALLOCATOR */
/*--------------------------------------------------------------------------*/

/* Don't forget to implement "init_allocator" and "release_allocator"! */
unsigned int init_allocator(unsigned int _basic_block_size, unsigned int _length) {
    _memory_start = (char *) malloc(_length);
    printf("\nThe starting address of memory is: %x\n", _memory_start);
    _memory_length = _length;
    _remainder_length = _length;
    _remainder_start = _memory_start;
    if (_remainder_length == 0)
        return 0;
    return _memory_length;
}

int release_allocator() {
    if (_memory_length != 0) {
        free(_memory_start);
        unsigned int _allocated_memory_length = _memory_length;
        _memory_length = 0;
        _remainder_length = 0;
        _memory_start = NULL;
        _remainder_start = NULL;
        return _allocated_memory_length;
    }
    return 0;
}

Addr my_malloc(size_t _length) {
    /* This preliminary implementation simply hands the call over the
     the C standard library!
     Of course this needs to be replaced by your implementation.
     */
    
    if (_remainder_length <= _length)
        return NULL;
    _remainder_start += _length;
    _remainder_length -= _length;
    return _remainder_start;
}

int my_free(Addr _a) {
    /* Same here! */
    //    free(_a);
    return 0;
}

