//
//  semaphore.c
//  MP2
//
//  Created by Susan Wang on 10/9/17.
//  Copyright © 2017 Suqian Wang. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "semaphore.H"

// initialize all private attribute
Semaphore::Semaphore(int _val) {
    this->value = _val;
    pthread_mutex_init(&this->m, NULL);
    pthread_cond_init(&this->c, NULL);
}

Semaphore::~Semaphore() {
    pthread_mutex_destroy(&this->m);
    pthread_cond_destroy(&this->c);
}

int Semaphore::P() {
    int error_code;
    error_code = pthread_mutex_lock(&this->m);
    if (error_code != 0) {
        return error_code;
    }
    this->value--;
    if (this->value < 0) {
        error_code = pthread_cond_wait(&this->c, &this->m);
        if (error_code != 0) {
            return error_code;
        }
    }
    error_code = pthread_mutex_unlock(&this->m);
    if (error_code != 0) {
        return error_code;
    }
    return 0;
}

int Semaphore::V() {
    int error_code;
    error_code = pthread_mutex_lock(&this->m);
    if (error_code != 0) {
        return error_code;
    }
    this->value++;
    if (this->value <= 0) {
        error_code = pthread_cond_signal(&this->c);
        if (error_code != 0) {
            return error_code;
        }
    }
    error_code = pthread_mutex_unlock(&this->m);
    if (error_code != 0) {
        return error_code;
    }
    return 0;
}
