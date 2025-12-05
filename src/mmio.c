#include "mmio.h"

void mmio_write(unsigned int reg, unsigned int data)
{
    *(volatile unsigned int*)reg = data;
}

unsigned int mmio_read(unsigned int reg)
{
    return *(volatile unsigned int*)reg;
}