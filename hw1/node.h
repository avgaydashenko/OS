#ifndef __NODE_H__
#define __NODE_H__

#include <stdint.h>

struct node {
    char* name;
    int capacity_level;
    uint64_t size;
    struct node* neighbor;
    struct node* child;
    void* file_start;
    uint8_t is_dir;
};

#endif /* __NODE_H__ */