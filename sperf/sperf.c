#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>

int channel[2];
char buf[4096];
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
    if (pipe(channel))
    {
        perror("Open Pipe Failed!");
        assert(0);
    }

    pid_t pid = fork();
    if (pid == 0)
    {
        /* child process */
        int trash = open("/dev/null", O_WRONLY);
        for (int i = 1; i < argc; i++)
            exec_argv[i] = argv[i];
        //dup2(trash, STDOUT_FILENO);
        //dup2(channel[1], STDERR_FILENO);
        execve("/usr/bin/strace", exec_argv, envp);
    }
    else
    {
        /* father process */
        //dup2(channel[0], STDIN_FILENO);
        /*
        while (fgets(buf, 4096, stdin) != NULL)
        {
            printf("fuck\n");
            printf("%s\n", buf);
        }
        */
        //getchar();
        printf("Finished!\n");
    }
}
