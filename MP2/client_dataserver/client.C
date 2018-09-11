//
//  client.cpp
//  MP2
//
//  Created by Susan Wang on 10/3/17.
//  Copyright © 2017 Suqian Wang. All rights reserved.
//

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <cassert>
#include <string>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>

#include <errno.h>
#include <unistd.h>

#include "reqchannel.H"

using namespace std;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- SUPPORT FUNCTIONS */
/*--------------------------------------------------------------------------*/

string int2string(int number) {
    stringstream ss;//create a stringstream
    ss << number;//add number to the stream
    return ss.str();//return a string with the contents of the stream
}

void print_time_diff(struct timeval * tp1, struct timeval * tp2) {
    /* Prints to stdout the difference, in seconds and museconds, between
     two timevals.
     */
    
    long sec = tp2->tv_sec - tp1->tv_sec;
    long musec = tp2->tv_usec - tp1->tv_usec;
    if (musec < 0) {
        musec += 1000000;
        sec--;
    }
    printf(" [sec = %ld, musec = %ld] ", sec, musec);
    
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {
    
    cout << "CLIENT STARTED:" << endl;
    
    /* -- Forks off a process */
    pid_t childID = fork();
    if (childID < 0) {
        perror("fork() error \n");
        exit(-1);
    }
    if (childID == 0) {
        /* child process, execute server process */
        execv("dataserver", NULL);
    }
    
    cout << "Establishing control channel... " << flush;
    RequestChannel chan("control", RequestChannel::CLIENT_SIDE);
    cout << "done." << endl;
    
    /* -- Start sending a sequence of requests */
    struct timeval tp_start; /* Used to compute elapsed time. */
    struct timeval tp_end;
    
    string reply1 = chan.send_request("hello");
    cout << "Reply to request 'hello' is '" << reply1 << "'" << endl;
    
    assert(gettimeofday(&tp_start, 0) == 0);
    string reply2 = chan.send_request("data Joe Smith");
    assert(gettimeofday(&tp_end, 0) == 0);
    cout << "Reply to request 'data Joe Smith' is '" << reply2 << "'" << endl;
    printf("invocation delay of request : ");
    print_time_diff(&tp_start, &tp_end);
    printf("\n");
    
    assert(gettimeofday(&tp_start, 0) == 0);
    string reply3 = chan.send_request("data Jane Smith");
    assert(gettimeofday(&tp_end, 0) == 0);
    cout << "Reply to request 'data Jane Smith' is '" << reply3 << "'" << endl;
    printf("invocation delay of request : ");
    print_time_diff(&tp_start, &tp_end);
    printf("\n");
    
    for(int i = 0; i < 100; i++) {
        string request_string("data TestPerson" + int2string(i));
        
        assert(gettimeofday(&tp_start, 0) == 0);
        string reply_string = chan.send_request(request_string);
        assert(gettimeofday(&tp_end, 0) == 0);
        cout << "reply to request " << i << ":" << reply_string << endl;
        printf("invocation delay of request : ");
        print_time_diff(&tp_start, &tp_end);
        printf("\n");
    }
    
    string reply4 = chan.send_request("quit");
    cout << "Reply to request 'quit' is '" << reply4 << endl;
    
    
    
    usleep(1000000);
}

