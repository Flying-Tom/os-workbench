#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>

int channel[2];
char buf[512];
int main(int argc, char *argv[], char *envp[])
{
    char *exec_argv[] = {
        "strace",
        //"-T",
        NULL,
    };
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
    pid_t pid = fork();
    int trash = open("/dev/null", O_WRONLY);
    if (pid == 0)
    {
        /* child process */
        printf("execve(%s, argv, envp)\n", "/bin/strace");
        assert(pipe(channel) >= 0);
        for (int i = 1; i < argc; i++)
            exec_argv[i] = argv[i];
        dup2(trash, STDERR_FILENO);
        //dup2(channel[1], STDOUT_FILENO);
        execve("/bin/strace", exec_argv, envp);
    }
    else
    {
        /* father process */
        printf("pid: %d\n", pid); //父进程中返回子进程的pid
        printf("father pid: %d\n", getpid());
        int nbytes;
        while ((nbytes = read(channel[0], buf, 512)) > 0)
            printf("%s\n", buf);
    }
}
