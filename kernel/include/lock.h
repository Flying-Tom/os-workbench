typedef struct
{
    int locked;
} lock_t;

#define LOCK_INIT() ((lock_t){.locked = 0})

void lock(lock_t *lk);
void unlock(lock_t *lk);