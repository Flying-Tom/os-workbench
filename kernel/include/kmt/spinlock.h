#ifndef SPINLOCK_H
#define SPINLOCK_H

struct spinlock
{
    const char *name;
    bool locked;
    int holder;

};


#endif