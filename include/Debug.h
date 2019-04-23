// Created by Graham Eger on 04/18/2019

#pragma once
#ifndef DEBUG_398_H
#define DEBUG_398_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif

//#define DEBUG

#ifdef DEBUG 
#define D(x) x
#else 
#define D(x)
#endif

#endif