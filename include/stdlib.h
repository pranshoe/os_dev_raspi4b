#ifndef STDLIB_H
#define STDLIB_H

void memcpy(void *dest, void *src, int bytes);
int memcmp(void *ptr1, void *ptr2, unsigned int size);
char* itoa(int n, int base);
char* utoa(unsigned int n, int base);

#endif