#ifndef MMIO_H
#define MMIO_H

void mmio_write(unsigned int reg, unsigned int data);
unsigned int mmio_read(unsigned int reg);

#endif