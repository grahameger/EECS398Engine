#pragma once
#ifndef MMAP_H_398
#define MMAP_H_398

#include <cstdlib>

void * mmapWrapper(int fd, size_t size, size_t offset);
int munmapWrapper(void * addr, size_t size);
int msyncWrapper(void * addr, size_t size);
void * streamMmapWrapper(int fd, size_t size);
int streamMunmapWrapper(void *addr, size_t size);
size_t getFileSize(int fd);
void extendFile(int fd, size_t newSize);


size_t roundUp(size_t toRound, size_t n);

#endif
