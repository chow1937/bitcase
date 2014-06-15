#ifndef _BCMEM_H
#define _BCMEM_H

#include <stdint.h>

void *bc_malloc(size_t size);
void bc_free(void *ptr);
size_t bc_used(void);

#endif
