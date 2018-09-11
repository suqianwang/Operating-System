/*
 File: simpleclient.C
 Author: R. Bettati
 Department of Computer Science
 Texas A&M University
 Date  : 2013/01/31
 Simple client main program for MP3 in CSCE 313
 */
/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/
#define NUM_PEOPLE 3
#define REPLY_RANGE 100
#define NUM_BINS 10
/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/
#include <cassert>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#include <map>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <pthread.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <iomanip>

#include <errno.h>
#include <unistd.h>

#include "reqchannel.H"
#include "boundedbuffer.H"

using namespace std;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/
string person[NUM_PEOPLE] = {"Joe Smith", "Jane Smith", "John Doe"};

int num_request;
int num_req_chan;
int buffer_size;

vector<int> joe_histogram(REPLY_RANGE);
vector<int> jane_histogram(REPLY_RANGE);
vector<int> john_histogram(REPLY_RANGE);

BoundedBuffer* req_buf;
BoundedBuffer* joe_buf;
BoundedBuffer* jane_buf;
BoundedBuffer* john_buf;

typedef struct rta {
    string name;
    int n;
    BoundedBuffer* wbb;
} req_thread_arg;

typedef struct eta {
    BoundedBuffer* wbb;
    //    map<string, BoundedBuffer*> hash_table;
    //    RequestChannel* channels[num_req_chan];
} event_thread_arg;

typedef struct sta {
    string name;
    //    BoundedBuffer* sbb;
} stat_thread_arg;

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

void print_histogram(vector<int> data, string name) {
    // create bins and sort data into bins
    vector<int> bin(NUM_BINS);
    for (int i = 0; i < NUM_BINS; i++) {
        for (int j = 0; j < (REPLY_RANGE/NUM_BINS); j++) {
            bin[i] += data[i*(REPLY_RANGE/NUM_BINS) + j];
        }
    }
    // formatting histogram's bins title and print bins' data
    cout << endl << " " << name << endl
    << setw(7) << "0-9" << setw(7) << "10-19" << setw(7) << "20-29"
    << setw(7) << "30-39" << setw(7) << "40-49" << setw(7) << "50-59" << setw(7) << "60-69"
    << setw(7) << "70-79" << setw(7) << "80-89" << setw(7) << "90-99" << endl <<endl;
    for (int i = 0; i < 10; i++) {
        cout << setw(7) << bin[i];
    }
    cout << endl << endl;
}

void* request_thread_function (void* rt_arg) {
    req_thread_arg* temp_arg = (req_thread_arg*) rt_arg;
    for (int i = 0; i < temp_arg->n; i++) {
        string request = "data " + temp_arg->name;
        temp_arg->wbb->deposit(request);
    }
}

void* event_thread_function (void* et_arg) {
    event_thread_arg* temp_arg = (event_thread_arg*) et_arg;
    
    // establish control channel
    RequestChannel chan("control", RequestChannel::CLIENT_SIDE);
    // send newthread request command through each channel to create new request channel
    RequestChannel* channels[num_req_chan];
    for (int i = 0; i < num_req_chan; i++) {
        string reply = chan.send_request("newthread");
        channels[i] = new RequestChannel(reply, RequestChannel::CLIENT_SIDE);
    }
    
    fd_set readset; // read file decriptors
    struct timeval te = {0, 10};
    int sel_return; // select function return value
    string name[num_req_chan];
    string request;
    int chan_counter = num_req_chan;
    int max_fd = -1;
    
    // fill all channels with requests
    for (int i = 0; i < num_req_chan; i++) {
        request =temp_arg->wbb->remove();
        channels[i]->cwrite(request);
        name[i] = request.substr(5);
    }
    
    for(;;) {
        // clear the readset and initialize it
        FD_ZERO(&readset);
        for(int i = 0; i < num_req_chan; i++)  {
            int read_i = channels[i]->read_fd();
            if(read_i > max_fd) {
                max_fd = read_i;
            }
            FD_SET(read_i, &readset);
        }
        
        // monitor when a file descriptor in the readset get a reponse
        sel_return = select(max_fd+1, &readset, NULL, NULL, &te);
        if(sel_return) {
            for (int i = 0; i < num_req_chan; i++) {
                if(FD_ISSET(channels[i]->read_fd(), &readset)) {
                    // read reply
                    string reply = channels[i]->cread();
                    // forward reply to statistic thread buffer
                    if (name[i] == person[0]) {
                        joe_buf->deposit(reply);
                    }
                    else if (name[i] == person[1]) {
                        jane_buf->deposit(reply);
                    }
                    else if (name[i] == person[2]) {
                        john_buf->deposit(reply);
                    }
                    // get a new request from the request buffer and send request through the empty channel
                    request = temp_arg->wbb->remove();
                    if (request == "done") {
                        chan_counter--;
                    }
                    else {
                        name[i] = request.substr(5);
                        channels[i]->cwrite(request);
                    }
                }
                
            }
        }
        if (chan_counter == 0) {
            break;
        }
    }
    
    // close request channel
    for(int i = 0; i < num_req_chan; i++) {
        channels[i]->send_request("quit");
    }
    chan.send_request("quit");
}

void* stats_thread_function (void* st_arg) {
    stat_thread_arg* temp_arg = (stat_thread_arg*) st_arg;
    string reply;
    
    for (int i = 0; i < num_request; i++) {
        if (temp_arg->name == person[0]) {
            reply = joe_buf->remove();
            joe_histogram[atoi(reply.c_str())] += 1;
        }
        else if (temp_arg->name == person[1]) {
            reply = jane_buf->remove();
            jane_histogram[atoi(reply.c_str())] += 1;
        }
        else if (temp_arg->name == person[2]) {
            reply = john_buf->remove();
            john_histogram[atoi(reply.c_str())] += 1;
        }
    }
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {
    // parsing command line arguments
    if (argc!= 7) {
        cout << "ERROR: incorrect number of arguments." << endl;
        return -1;
    }
    for (int i = 0; i < argc; i++) {
        if (argv[i] == string("-n")) {
            num_request = atoi(argv[i+1]);
        }
        if (argv[i] == string("-b")) {
            buffer_size = atoi(argv[i+1]);
        }
        if (argv[i] == string("-w")) {
            num_req_chan = atoi(argv[i+1]);
        }
    }
    
    pthread_t request_threads[NUM_PEOPLE];
    pthread_t event_thread;
    pthread_t stats_threads[NUM_PEOPLE];
    
    req_buf = new BoundedBuffer(buffer_size);
    joe_buf = new BoundedBuffer(buffer_size);
    jane_buf = new BoundedBuffer(buffer_size);
    john_buf = new BoundedBuffer(buffer_size);
    
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork() failed\n");
        exit(-1);
    }
    if (pid == 0) {
        execv("dataserver", NULL);
    }
    else {
        cout << "CLIENT STARTED:" << endl;
        
        cout << "Establishing control channel... " << flush;
        RequestChannel chan("control", RequestChannel::CLIENT_SIDE);
        cout << "done." << endl;
        
        timeval begin, end;
        gettimeofday(&begin, NULL);
        
        
        // creating request thread argument and request threads
        req_thread_arg rt_arg[NUM_PEOPLE];
        for (int i = 0; i < NUM_PEOPLE; i++) {
            rt_arg[i].name = person[i];
            rt_arg[i].n = num_request;
            rt_arg[i].wbb = req_buf;
        }
        cout << endl << endl << "Creating request threads..." << endl << endl;
        for (int i = 0; i < NUM_PEOPLE; i++) {
            pthread_create(&request_threads[i], NULL, request_thread_function, (void*)(&rt_arg[i]));
        }
        cout << endl << endl << "done." << endl << endl;
        
        
        // creating event thread argument and event threads
        event_thread_arg et_arg;
        et_arg.wbb = req_buf;
        cout << endl << endl << "Creating event handler..." << endl << endl;
        pthread_create(&event_thread, NULL, event_thread_function, (void*)(&et_arg));
        cout << endl << endl << "done." << endl << endl;
        
        
        // creating statistic thread argument and statistc threads
        stat_thread_arg st_arg[NUM_PEOPLE];
        for (int i = 0; i < NUM_PEOPLE; i++) {
            st_arg[i].name = person[i];
        }
        cout << endl << endl << "Creating statistic threads..." << endl << endl;
        for (int i = 0; i < NUM_PEOPLE; i++) {
            pthread_create(&stats_threads[i], NULL, stats_thread_function, (void*)(&st_arg[i]));
        }
        cout << endl << endl << "done." << endl << endl;
        
        
        // waiting for thread to finish
        for (int i = 0; i < NUM_PEOPLE; i++) {
            pthread_join(request_threads[i], NULL);
        }
        for (int i = 0; i < num_req_chan; i++) {
            req_buf->deposit("done");
        }
        pthread_join(event_thread, NULL);
        for (int i = 0; i < NUM_PEOPLE; i++) {
            pthread_join(stats_threads[i], NULL);
        }
        
        gettimeofday(&end, NULL);
        
        chan.send_request("quit");
        usleep(1000000);
        system("rm -f fifo*");
        
        // print histogram and information
        print_histogram(joe_histogram, person[0]);
        print_histogram(jane_histogram, person[1]);
        print_histogram(john_histogram, person[2]);
        cout << "Total number of requests: " << num_request*3 << endl;
        cout << "Total number of worker threads: " << num_req_chan << endl;
        cout << "Total request time: " << end.tv_sec-begin.tv_sec << " sec " << end.tv_usec-begin.tv_usec << " musec" << endl;
    }
}

