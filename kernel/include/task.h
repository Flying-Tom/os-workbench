#ifndef TASK_H
#define TASK_H

#define STACK_SIZE 8192

struct task {

    const char* name;
    enum task_status {
        SLEEP,
        DEAD,
    } status;

    Context* context;
    int cpu;

    char stack[STACK_SIZE];
};

typedef struct task task_t;

#endif