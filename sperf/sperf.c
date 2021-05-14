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
} syscall_rec[512], syscall_temp;

int syscall_num = -1;
double total_exec_time = 0;

void findtimemax()
{
    double syscall_time_max = 0;
    int syscall_id = 0;
    for (int i = 0; i <= syscall_num; i++)
    {
        if (syscall_rec[i].time > syscall_time_max)
        {
            syscall_time_max = syscall_rec[i].time;
            syscall_id = i;
        }
    }
    printf("%s(%.0lf%%)\n", syscall_rec[syscall_id].name, 100 * syscall_rec[syscall_id].time / total_exec_time);
    syscall_rec[syscall_id].time = 0;
}

void child(int pipe, int exec_argc, char *argv[], char *exec_envp[])
{
    char *exec_argv[exec_argc + 10];
    exec_argv[0] = "strace";
    exec_argv[1] = "-T";
    int trash = open("/dev/null", O_WRONLY);

    char test[2048];
    strcpy(test, getenv("PATH"));
    char *testp;
    testp = strtok(test, ":");
    while (testp != NULL)
    {
        printf("%s\n", testp);
        testp = strtok(NULL, ":");
    }

    memcpy(exec_argv + 2, argv + 1, exec_argc * sizeof(char *));
    dup2(trash, STDOUT_FILENO);
    dup2(pipe, STDERR_FILENO);
    execve("strace", exec_argv, exec_envp);
}

void parent(int pipe)
{
    dup2(pipe, STDIN_FILENO);
    char syscall_name[32];
    double syscall_time = 0;
    int syscall_rec_cnt = 0;

    while (fgets(buf, sizeof(buf), stdin) != NULL)
    {
        //printf("%s\n", buf);
        memset(syscall_name, '\0', sizeof(syscall_name));
        sscanf(buf, "%[^(]%*[^<]<%lf>\n", syscall_name, &syscall_time);
        memset(buf, '\0', sizeof(buf));

        for (syscall_rec_cnt = 0; syscall_rec_cnt <= syscall_num; syscall_rec_cnt++)
        {
            if (strcmp(syscall_rec[syscall_rec_cnt].name, syscall_name) == 0)
            {
                //printf("%s\n", syscall_rec[syscall_rec_cnt].name);
                syscall_rec[syscall_rec_cnt].time += syscall_time;
                break;
            }
        }
        if (syscall_rec_cnt > syscall_num)
        {
            //printf("%s\n", syscall_rec[syscall_rec_cnt].name);
            syscall_num++;
            memcpy(syscall_rec[syscall_rec_cnt].name, syscall_name, sizeof(syscall_name));
            syscall_rec[syscall_rec_cnt].time = syscall_time;
        }
    }
    syscall_num--;
    for (int i = 0; i <= syscall_num; i++)
    {
        total_exec_time += syscall_rec[i].time;
    }

    for (int i = 0; i <= 5; i++)
        findtimemax();
}

int main(int argc, char *argv[], char *envp[])
{
    assert(argc >= 2);
    if (pipe(channel))
    {
        perror("Open Pipe Failed!");
        assert(0);
    }
    /*
    for (int i = 0; i < 5; i++)
        printf("%s\n", envp[i]);
    */
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
