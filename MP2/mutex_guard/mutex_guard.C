//
//  mutex_guard.c
//  MP2
//
//  Created by Susan Wang on 10/9/17.
//  Copyright © 2017 Suqian Wang. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "mutex_guard.H"

MutexGuard::MutexGuard(Mutex & m){
    this->m = &m;
    m.Lock();
}

MutexGuard::~MutexGuard() {
    this->m->Unlock();
}
