#ifndef MEM_H
#define MEM_H
#include <types.h>
uint32_t kmalloc(uint32_t sz);

uint32_t kmalloc_p(uint32_t sz, uint32_t *phys);

uint32_t kmalloc_ap(uint32_t sz, uint32_t *phys);

uint32_t kmalloc_a(uint32_t sz);

#endif
