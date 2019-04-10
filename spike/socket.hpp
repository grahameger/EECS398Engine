//
//  socket.hpp
//  pthread_wrapper
//
//  Created by Graham Eger on 2/14/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//

#ifndef socket_hpp_398
#define socket_hpp_398

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <signal.h>
#include <sys/signalfd.h>

#include <tls.h>

#include "http.hpp"

namespace search {

    enum State
    {
        START,
        DNSWAITING,
        CONNECTWAITING,
        HAVETCP,
        SSLWAITING,
        HAVECONNECTION,
        SENDWAITING,
        RESPONSEWAITING,
        RECVWAITING,
        HAVEFILE
    };

    class TLSConfig { 
        TLSConfig() {
            tls_init();
            config = tls_config_new();
            // TODO: maybe setup root ceritificates?

        }
        tls_config * config;
    };


    class Socket {
    public:
        void event();

        Socket() {

        }
        ~Socket() {

        }
    private:
        int fd; 
        char * buffer;
        ssize_t bytes_received;        
        ssize_t buffer_size;
        HTTPRequest request;
        static TLSConfig tlsConfig;
        State state;
        tls * tlsClient;
        ssize_t total_size;
        bool haveHeader;
        bool haveContentLength;
    private:
        // state change functions
        void DNSDispatch();
        void DNSReceive();
        void TCPDispatch();
        void TCPHandshake();
        void TLSDispatch();
        void TLSHandshake();
        void SendRequest();
        void RecvResponse();
        void ProcessResponse();

    private: // DNS helper functions
        static int signalFdSetup();
        void signalFdRead();
        void createServerStruct();
    };
}

#endif