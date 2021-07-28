void vfs_init() {

};

int write(int fd, void* buf, int count) {

};
int read(int fd, void* buf, int count) {

};
int close(int fd) {

};
int open(const char* pathname, int flags) {

};

int lseek(int fd, int offset, int whence) {

};
int link(const char* oldpath, const char* newpath) {

};
int unlink(const char* pathname) {

};
int fstat(int fd, struct ufs_stat* buf) {

};
int mkdir(const char* pathname) {

};
int chdir(const char* path) {

};
int dup(int fd) {

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