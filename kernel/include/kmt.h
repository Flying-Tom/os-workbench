#ifndef KMT_H
#define KMT_H

#include <common.h>
#include <kmt/semaphore.h>
#include <kmt/spinlock.h>
#include <kmt/task.h>

static Context* kmt_context_save(Event e, Context* c);
static Context* kmt_schedule(Event e, Context* c);

#endif