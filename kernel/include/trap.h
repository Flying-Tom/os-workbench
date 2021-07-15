#ifndef TRAP_H
#define TRAP_H

#include <common.h>

#define SEQ_MIN 0
#define SEQ_MAX 512

struct trap_handler {
    int seq;
    int event;
    handler_t handler;
    int valid;
};

typedef trap_handler trap_handler_t;

#endif
