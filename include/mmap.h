#pragma once
#ifndef MMAP_H_398
#define MMAP_H_398

#include <cstdlib>

void * mmapWrapper(int fd, size_t size, size_t offset);
int munmapWrapper(void * addr, size_t size);
size_t fileSize(int fd);
void extendFile(int fd, size_t newSize);

#endif