#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

int main(int argc, char *argv[])
{
    char *exec_envp[] = {
        "PATH=/bin",
        NULL,
    };
    /*
    char *exec_argv[] = {
        "strace",
        "ls",
        NULL,
    };
    char *exec_envp[] = {
        "PATH=/bin",
        NULL,
    };
    execve("strace", exec_argv, exec_envp);
    execve("/bin/strace", exec_argv, exec_envp);
    execve("/usr/bin/strace", exec_argv, exec_envp);
    perror(argv[0]);
    exit(EXIT_FAILURE);
    */

    assert(argc >= 2);
    pid_t pid;

    pid = fork();
    if (pid == 0)
    {
        /* child process */
        execve(argv[1], argv + 2, exec_envp);
        printf("argv[0]:%s\n", argv[0]);
    }
    else
    {
        printf("pid: %d\n", pid); //父进程中返回子进程的pid
        printf("father pid: %d\n", getpid());
    }
}
