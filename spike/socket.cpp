//
//  socket.hpp
//  pthread_wrapper
//
//  Created by Graham Eger on 3/13/19.
//  Copyright © 2019 Graham Eger. All rights reserved.
//

#include "socket.hpp"

namespace search {
    void Socket::event() {
        switch (state) {
            case START:
                DNSDispatch();
                break;
            case DNSWAITING:
                DNSReceive();
                break;
            case HAVETCP:
                TCPHandshake();
                break;
            case SSLWAITING:
                TLSDispatch();
                break;
            case HAVECONNECTION:
                TLSHandshake();
                break;
            case SENDWAITING:
                SendRequest();
                break;
            case RESPONSEWAITING:
                RecvResponse();
                break;
            case HAVEFILE:
                ProcessResponse();
                break;
        }
    }

    // returns the file descriptor that will be used
    // with epoll to signal when our async DNS request
    // has been processed
    int Socket::signalFdSetup() {
        int sfd;
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGRTMIN);
        sigprocmask(SIG_BLOCK, &mask, NULL);
        sfd = signalfd(-1, &mask, 0);
        return sfd; 
    }

    void Socket::signalFdRead() {
        ssize_t size;
        signalfd_siginfo fdsi;
        gaicb * host;

        while ((size = read(fd, &fdsi, sizeof(signalfd_siginfo))) > 0) {
            if (size != sizeof(signalfd_siginfo)) {
                return; // bad
            }
            host = fdsi.ssi_ptr; // the pointer passed to the sigevent structure
            // the rest is in the host->ar_result member
        }
    }

    void Socket::createServerStruct() {
        struct addrinfo * p, *result;
        p = result = NULL;
        int sockfd; 

        for (p = result; p != NULL; p = p->ai_next) {
            
        }
    }

    // dispatches an async DNS request and adds the waiting fd
    // to the eventing mechanism of choice. Will signal and have
    // a corresponding Socket struct which will change state when
    // the DNS query has been processed.
    void Socket::DNSDispatch() {
        int sfd = signalFdSetup();

    }
}

