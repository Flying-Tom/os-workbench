#include <vfs.h>

void vfs_init() {

};

int write(int fd, void* buf, int count)
{
    return 0;
};
int read(int fd, void* buf, int count)
{
    return 0;
};
int close(int fd)
{
    return 0;
};
int open(const char* pathname, int flags)
{
    return 0;
};

int lseek(int fd, int offset, int whence)
{
    return 0;
};
int link(const char* oldpath, const char* newpath)
{
    return 0;
};
int unlink(const char* pathname)
{
    return 0;
};
int fstat(int fd, struct ufs_stat* buf)
{
    return 0;
};
int mkdir(const char* pathname)
{
    return 0;
};
int chdir(const char* path)
{
    return 0;
};
int dup(int fd)
{
    return 0;
};

MODULE_DEF(vfs) = {
    .init = vfs_init,
    .write = write,
    .read = read,
    .close = close,
    .open = open,
    .lseek = lseek,
    .link = link,
    .unlink = unlink,
    .fstat = fstat,
    .mkdir = mkdir,
    .chdir = chdir,
    .dup = dup,

};