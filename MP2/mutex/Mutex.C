//
//  mutex.c
//  MP2
//
//  Created by Susan Wang on 10/4/17.
//  Copyright © 2017 Suqian Wang. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "mutex.H"

/* -- CONSTRUCTOR/DESTRUCTOR */

Mutex::Mutex(){
    int error = pthread_mutex_init(&this->m, NULL);
    if ( error!= 0) {
        printf("\nmutex initialization failed with error code %d\n", error);
    }
}

Mutex::~Mutex() {
    int error = pthread_mutex_destroy(&this->m);
    if ( error!= 0) {
        printf("\nmutex destruction failed with error code %d\n", error);
    }
}

/* -- MUTEX OPERATIONS */

void Mutex::Lock() {
    int error = pthread_mutex_lock(&this->m);
    if ( error!= 0) {
        printf("\nmutex lock failed with error code %d\n", error);
    }
}

void Mutex::Unlock() {
    int error = pthread_mutex_unlock(&this->m);
    if ( error!= 0) {
        printf("\nmutex unlock failed with error code %d\n", error);
    }
}


