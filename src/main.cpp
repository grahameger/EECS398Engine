// Created by Graham Eger on 02/01/2019

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <thread>
#include <mutex>
//#include "index.h"
#include "String.h"
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <deque>
#include "PersistentHashMap.h"
#include "Parser.hpp"
#include "threading.h"
#include "index.h"
// used to gracefully shut down and run all of our destructors
volatile sig_atomic_t keep_running = 1;

static void sig_handler(int _)
{
    (void)_;
    keep_running = 0;
}

static const char startFile[] = "/data/crawl/seedlist.url";

int main(int argc, char *argv[]) {

   std::deque<Doc_object> docList;
   threading::Mutex queueLock;
   threading::ConditionVariable cv;
   Index index(&docList, &queueLock, &cv);
   Doc_object d;
   Vector<Index_object> anchor;
   d.doc_url = String("github.com");
   d.num_slash_in_url = 1;
   d.Links.push_back(String("abc.com"));
   Index_object i;
   i.word = String("a");
   i.type = String("t");
   i.position = 0;
   d.Words.push_back(i);
   i.word = String("b");
   i.type = String("b");
   i.position = 1;
   d.Words.push_back(i);
   i.word = String("c");
   i.type = String("b");
   i.position = 2;
   d.Words.push_back(i);
   i.word = String("co");
   i.type = String("b");
   i.position = 3;
   d.Words.push_back(i);
   i.word = String("iabc");
   i.type = String("a");
   i.position = 4;
   anchor.push_back(i);
   d.Words.push_back(i);
   i.word = String("pows");
   i.type = String("a");
   i.position = 5;
   d.Words.push_back(i);
   anchor.push_back(i);
   d.anchor_words.push_back(anchor);
   d.url.push_back(String("github"));
   d.url.push_back(String("398"));
   queueLock.lock();
   docList.push_back(d);
   queueLock.unlock();
   while(true){}
}
