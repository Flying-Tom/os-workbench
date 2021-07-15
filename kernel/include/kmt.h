#ifndef KMT_H
#define KMT_H

#include <common.h>
#include <sem.h>
#include <spinlock.h>
#include <task.h>

task_t* cur_task[MAX_CPU_NUM];
task_t* tasks[MAX_CPU_NUM][MAX_TASK_NUM];

#endif