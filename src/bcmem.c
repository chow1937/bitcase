#include <stdio.h>
#include <stdlib.h>


static size_t used = 0;

/*A function to encapsulate the standard malloc*/
void *bc_malloc(size_t size) {
    void *ptr;

    if (ptr=malloc(size)) {
        /*Add the new alloc memory bytes*/
        used += size;
    }
    return ptr;
}

/*A function to encapsulate the standard free*/
void bc_free(void *ptr) {
    size_t header_size = sizeof(size_t);
    size_t size;

    size = *(size_t*)(ptr - header_size);
    free(ptr);
    used -= size;
}

size_t bc_used(void) {
    return used;
}
