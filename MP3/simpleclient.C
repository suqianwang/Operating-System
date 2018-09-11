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
int num_worker_thread;
int buffer_size;

int joe_counter = 0;
int jane_counter = 0;
int john_counter = 0;

vector<int> joe_histogram(100);
vector<int> jane_histogram(100);
vector<int> john_histogram(100);

BoundedBuffer* req_buf;
BoundedBuffer* joe_buf;
BoundedBuffer* jane_buf;
BoundedBuffer* john_buf;



typedef struct rta {
    string name;
    int n;
    BoundedBuffer* wbb;
} req_thread_arg;

typedef struct wta {
    BoundedBuffer* wbb;
//    map<string, BoundedBuffer*> hash_table;
    RequestChannel* channel;
} work_thread_arg;

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
    vector<int> bin(10);
    for (int i = 0; i < bin.size(); i++) {
        for (int j = 0; j < bin.size(); j++) {
            bin[i] += data[i*10 + j];
        }
    }
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
    temp_arg->wbb->deposit("done");
}

//BoundedBuffer* look_up(string reqest, map<string, BoundedBuffer*> hash_table) {
//    string name = reqest.substr(5);
//    return hash_table.find(name)->second;
//}

void* worker_thread_function (void* wt_arg) {
    Semaphore req_thread_m(1);
    work_thread_arg* temp_arg = (work_thread_arg*) wt_arg;
    int req_thread_counter = NUM_PEOPLE;
    for (;;) {
        string request =temp_arg->wbb->remove();
//        if (request == "done") {
//            req_thread_m.P();
//            req_thread_counter--;
//            if (req_thread_counter == 0) {
//                for (int i = 0; i < num_worker_thread; i++) {
//                    temp_arg->wbb->deposit("quit");
//                }
//            }
//            req_thread_m.V();
//            if (request == "quit") {
//                break;
//            }
//        }
        if (request == "done") {
            break;
        }
        string reply = temp_arg->channel->send_request(request);
        string name = request.substr(5);
        if (name == person[0]) {
            joe_buf->deposit(reply);
        }
        else if (name == person[1]) {
            jane_buf->deposit(reply);
        }
        else if (name == person[2]) {
            john_buf->deposit(reply);
        }
    }
    temp_arg->channel->send_request("quit");
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
            num_worker_thread = atoi(argv[i+1]);
        }
    }
    
    pthread_t request_threads[NUM_PEOPLE];
    pthread_t worker_threads[num_worker_thread];
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
        
        
        work_thread_arg wt_arg[num_worker_thread];
        for (int i = 0; i < num_worker_thread; i++) {
            string reply = chan.send_request("newthread");
            RequestChannel* req_channel = new RequestChannel(reply, RequestChannel::CLIENT_SIDE);
            wt_arg[i].wbb = req_buf;
            wt_arg[i].channel = req_channel;
        }
        
        cout << endl << endl << "Creating worker threads..." << endl << endl;
        for (int i = 0; i < num_worker_thread; i++) {
            pthread_create(&worker_threads[i], NULL, worker_thread_function, (void*)(&wt_arg[i]));
        }
        cout << endl << endl << "done." << endl << endl;
        
        stat_thread_arg st_arg[NUM_PEOPLE];
        for (int i = 0; i < NUM_PEOPLE; i++) {
            st_arg[i].name = person[i];
        }
        
//        waitpid(pid, NULL, 0);
        cout << endl << endl << "Creating statistic threads..." << endl << endl;
        
        for (int i = 0; i < NUM_PEOPLE; i++) {
            pthread_create(&stats_threads[i], NULL, stats_thread_function, (void*)(&st_arg[i]));
        }
        cout << endl << endl << "done." << endl << endl;
        
        for (int i = 0; i < NUM_PEOPLE; i++) {
            pthread_join(request_threads[i], NULL);
        }
        for (int i = 0; i < num_worker_thread; i++) {
            req_buf->deposit("done");
        }
        for (int i = 0; i < num_worker_thread; i++) {
            pthread_join(worker_threads[i], NULL);
        }
        for (int i = 0; i < NUM_PEOPLE; i++) {
            pthread_join(stats_threads[i], NULL);
        }
        
        gettimeofday(&end, NULL);
        
        chan.send_request("quit");
        usleep(1000000);
        
        print_histogram(joe_histogram, person[0]);
        print_histogram(jane_histogram, person[1]);
        print_histogram(john_histogram, person[2]);
        
        cout << "Total number of requests: " << num_request*3 << endl;
        cout << "Total number of worker threads: " << num_worker_thread << endl;
        cout << "Total request time: " << end.tv_sec-begin.tv_sec << " sec " << end.tv_usec-begin.tv_usec << " musec" << endl;
    }
}

