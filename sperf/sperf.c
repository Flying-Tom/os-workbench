#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

int channel[2];
char buf[4096];

struct Syscall
{
    char name[32];
    double time;
} syscall_rec[512];

int syscall_num = -1;

void child(int pipe, int exec_argc, char *argv[], char *exec_envp[])
{
    char *exec_argv[exec_argc + 10];
    exec_argv[0] = "strace";
    exec_argv[1] = "-T";
    int trash = open("/dev/null", O_WRONLY);

    memcpy(exec_argv + 2, argv + 1, exec_argc * sizeof(char *));
    dup2(trash, STDOUT_FILENO);
    dup2(pipe, STDERR_FILENO);
    execve("/usr/bin/strace", exec_argv, exec_envp);
}

void parent(int pipe)
{
    printf("pipe:%d\n", pipe);
    char syscall_name[32];
    double syscall_time;
    int syscall_rec_cnt;

    while (read(pipe, buf, sizeof(buf)))
    {
        //printf("%s", buf);
        memset(syscall_name, '\0', sizeof(syscall_name));
        sscanf(buf, "%[^(]%*[^<]<%lf>\n", syscall_name, &syscall_time);
        for (syscall_rec_cnt = 0; syscall_rec_cnt <= syscall_num; syscall_rec_cnt++)
        {
            if (strcmp(syscall_rec[syscall_rec_cnt].name, syscall_name) == 0)
            {
                printf("%s\n", syscall_rec[syscall_rec_cnt].name);
                syscall_rec[syscall_rec_cnt].time += syscall_time;
                break;
            }
        }
        if (syscall_rec_cnt > syscall_num)
        {
            printf("%s\n", syscall_rec[syscall_rec_cnt].name);
            syscall_num++;
            memcpy(syscall_rec[syscall_rec_cnt].name, syscall_name, sizeof(syscall_name));
            syscall_rec[syscall_rec_cnt].time += syscall_time;
        }
        memset(buf, '\0', sizeof(buf));
    }
    printf("Finished!\n");
}

int main(int argc, char *argv[], char *envp[])
{
    assert(argc >= 2);
    if (pipe(channel))
    {
        perror("Open Pipe Failed!");
        assert(0);
    }
    printf("channel[0]:%d channel[1]:%d\n", channel[0], channel[1]);

    pid_t pid = fork();
    if (pid == 0)
    {
        /* child process */
        close(channel[0]);
        child(channel[1], argc - 1, argv, envp);
    }
    else
    {
        /* parent process */
        close(channel[1]);
        parent(channel[0]);
        close(channel[0]);
    }
    return 0;
}
