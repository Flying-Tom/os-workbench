#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>

int channel[2];
char buf[4096];

void child(int pipe, int argc, char *argv[], char *exec_envp[])
{
    char *exec_argv[argc + 2];
    int trash = open("/dev/null", O_WRONLY);

    memcpy(exec_argv + 2, argv + 1, (argc - 2) * sizeof(char *));
    for (int i = 0; i < sizeof(exec_argv) / sizeof(char *); i++)
        printf("%s ", exec_argv[i]);

    printf("\n");
    dup2(trash, STDOUT_FILENO);
    dup2(trash, STDERR_FILENO);
    execve("strace", exec_argv, exec_envp);
}

int main(int argc, char *argv[], char *envp[])
{
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
        child(channel[1], argc, argv, envp);
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
