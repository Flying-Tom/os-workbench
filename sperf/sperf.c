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
    pid_t pid = fork();
    int trash = open("/dev/null", O_WRONLY);
    if (pid == 0)
    {
        /* child process */
        printf("execve(%s, argv, envp)\n", "/bin/strace");
        assert(pipe(channel) == 0);
        close(channel[0]);
        for (int i = 1; i < argc; i++)
            exec_argv[i + 1] = argv[i];
        dup2(trash, STDOUT_FILENO);
        dup2(channel[1], STDERR_FILENO);
        close(channel[1]);
        execve("/bin/strace", exec_argv, envp);
    }
    else
    {
        /* father process */
        dup2(channel[0], STDIN_FILENO);
        /*
        while (fgets(buf, 4096, stdin) != NULL)
        {
            printf("fuck\n");
            printf("%s\n", buf);
        }
        */
        scanf("%s", buf);
        printf("Finished!\n");
    }
}
