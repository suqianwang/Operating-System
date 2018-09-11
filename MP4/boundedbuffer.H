//
//  boundedbuffer.H
//  MP3
//
//  Created by Susan Wang on 10/29/17.
//  Copyright © 2017 Suqian Wang. All rights reserved.
//

#ifndef _boundedbuffer_H_
#define _boundedbuffer_H_

#include "semaphore.H"
#include <string>
#include <queue>

class BoundedBuffer {
private:
    Semaphore * full;
    Semaphore * empty;
    Semaphore * mutex;
    
    int size;
    queue<string> buffer;
    
public:
    BoundedBuffer() {}
    
    BoundedBuffer(int _size) {
        this->size = _size;
        full = new Semaphore(0);
        empty = new Semaphore(size);
        mutex = new Semaphore(1);
    }
    
    ~BoundedBuffer() {
        delete full;
        delete empty;
        delete mutex;
    }
    
    void deposit(string item) {
        empty->P();
        
        mutex->P();
        buffer.push(item);
        mutex->V();
        
        full->V();
    }
    
    string remove() {
        full->P();
        
        mutex->P();
        string item = buffer.front();
        buffer.pop();
        mutex->V();
        
        empty->V();
        return item;
    }
};

#endif /* boundedbuffer_h */
