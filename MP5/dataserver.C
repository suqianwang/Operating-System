/*
 File: dataserver.C
 
 Author: R. Bettati
 Department of Computer Science
 Texas A&M University
 Date  : 2012/07/16
 
 Dataserver main program for MP3 in CSCE 313
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <cassert>
#include <cstring>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include "NetworkRequestChannel.H"

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
/* VARIABLES */
/*--------------------------------------------------------------------------*/

static int nthreads = 0;
int backlog;
unsigned short port_no;

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

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- THREAD FUNCTIONS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- INDIVIDUAL REQUESTS */
/*--------------------------------------------------------------------------*/

void process_hello(NetworkRequestChannel & _channel, const string & _request) {
    _channel.cwrite("hello to you too");
}

void process_data(NetworkRequestChannel & _channel, const string &  _request) {
    usleep(1000 + (rand() % 5000));
    //_channel.cwrite("here comes data about " + _request.substr(4) + ": " + int2string(random() % 100));
    _channel.cwrite(int2string(rand() % 100));
}

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- THE PROCESS REQUEST LOOP */
/*--------------------------------------------------------------------------*/

void process_request(NetworkRequestChannel & _channel, const string & _request) {
    
    if (_request.compare(0, 5, "hello") == 0) {
        process_hello(_channel, _request);
    }
    else if (_request.compare(0, 4, "data") == 0) {
        process_data(_channel, _request);
    }
    else {
        _channel.cwrite("unknown request");
    }
    
}

void* connection_handler(void* arg) {
    NetworkRequestChannel *channel = (NetworkRequestChannel*)arg;
    
    for(;;) {
        string request = channel->cread();
        
        if (request.compare("quit") == 0) {
            channel->cwrite("bye");
            usleep(10000);          // give the other end a bit of time.
            break;                  // break out of the loop;
        }
        
        process_request(*channel, request);
    }
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {
    int opt = 0;
    while((opt = getopt(argc, argv, "p:b:")) != -1) {
        switch(opt) {
            case 'b':
                backlog = atoi(optarg);
                break;
            case 'p':
                port_no = atoi(optarg);
                break;
            default:
                backlog = 128;
                break;
        }
    }
    NetworkRequestChannel server(port_no, connection_handler, backlog);
}

