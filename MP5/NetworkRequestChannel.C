//
//  NetworkRequestChannel.C
//  MP5
//
//  Created by Susan Wang on 12/7/17.
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
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sstream>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "NetworkRequestChannel.H"

using namespace std;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR */
/*--------------------------------------------------------------------------*/

NetworkRequestChannel::NetworkRequestChannel(const string _server_host_name, const unsigned short _port_no) {
    
    stringstream ss;
    ss << _port_no;
    string port = ss.str();
    
    int sfd;
    struct addrinfo serverIn, *res;
    int status;
    
    // set up structure
    memset(&serverIn, 0, sizeof(serverIn));
    serverIn.ai_family = AF_INET;
    serverIn.ai_socktype = SOCK_STREAM;
    
    status = getaddrinfo(_server_host_name.c_str(), port.c_str(), &serverIn, &res);
    if(status != 0) {
        cerr << gai_strerror(status) << endl;
    }
    
    // make a socket
    sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sfd < 0) {
        cerr << "cannot create socket\n";
    }
    
    // connect socket
    if (connect(sfd, res->ai_addr, res->ai_addrlen) < 0) {
        cerr << "cannot connect to " << _server_host_name << ": " << _port_no << endl;
    }
    fd = sfd;
}

NetworkRequestChannel::NetworkRequestChannel(const unsigned short _port_no, void* (*connection_handler)(void*), int backlog) {
    stringstream ss;
    ss << _port_no;
    string port = ss.str();
    
    struct addrinfo *servInfo, serverIn;
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    int sfd, new_fd;

    // load up address structs
    memset(&serverIn, 0, sizeof serverIn);
    serverIn.ai_family = AF_INET;
    serverIn.ai_socktype = SOCK_STREAM;
    serverIn.ai_flags = AI_PASSIVE;
    
    int status;
    status = getaddrinfo(NULL, port.c_str(), &serverIn, &servInfo);
    if (status != 0) {
        cerr << gai_strerror(status) << endl;
    }
    
    // make a socket, bind it, and listen on it
    sfd = socket(servInfo->ai_family, servInfo->ai_socktype, servInfo->ai_protocol);
    if (sfd < 0) {
        cerr << "cannot create socket\n";
    }
    
    fd = sfd;
    
    if (::bind(sfd, servInfo->ai_addr, servInfo->ai_addrlen) < 0) {
        close(fd);
        cerr << "cannot bind socket\n";
        exit(1);
    }
    
    freeaddrinfo(servInfo);
    
    if (listen(sfd, backlog) < 0) {
        cerr << "listening error\n";
        exit(1);
    }
    
    // accept an incoming connection
    while(1) {
        // accpt an incoming connection
        addr_size = sizeof(their_addr);
        new_fd = accept(sfd, (struct sockaddr *)&their_addr, &addr_size);
        if (new_fd < 0) {
            perror("error on accept");
        }
        
        int *newsfds = new int;
        *newsfds = new_fd;
        pthread_t id;
        pthread_create(&id, NULL, connection_handler, newsfds);
    }
}

NetworkRequestChannel::~NetworkRequestChannel() {
    close(fd);
}

/*--------------------------------------------------------------------------*/
/* READ/WRITE FROM/TO REQUEST CHANNELS  */
/*--------------------------------------------------------------------------*/

const int MAX_MESSAGE = 255;

string NetworkRequestChannel::send_request(string _request) {
    cwrite(_request);
    string s = cread();
    return s;
}

string NetworkRequestChannel::cread() {
    char buf[MAX_MESSAGE];
    if (recv(fd, buf, MAX_MESSAGE, 0) < 0) {
        perror("Error receiving message");
    }
    string s = buf;
    return s;
}

int NetworkRequestChannel::cwrite(string _msg) {
    if (_msg.length() >= MAX_MESSAGE) {
        cerr << "Message too long for Channel!\n";
        return -1;
    }
    const char * s = _msg.c_str();
    if (send(fd, s, strlen(s)+1, 0) < 0) {
        perror("Error sending message");
    }
    return 0;
}


