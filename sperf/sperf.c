#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>

time_t now;

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
    char exec_path[128], path[2048];

    exec_argv[0] = "strace";
    exec_argv[1] = "-T";
    memcpy(exec_argv + 2, argv + 1, exec_argc * sizeof(char *));
    exec_argv[exec_argc + 2] = ">";
    exec_argv[exec_argc + 3] = "/dev/null";
    exec_argv[exec_argc + 4] = "2>&1";
    exec_argv[exec_argc + 5] = NULL;

    int trash = open("/dev/null", O_WRONLY);
    assert(trash > 0);
    dup2(trash, STDOUT_FILENO);
    dup2(pipe, STDERR_FILENO);

    strcpy(path, getenv("PATH"));

    char *temp = strtok(path, ":");
    strcpy(exec_path, temp);
    strcat(exec_path, "/strace");

    while (execve(exec_path, exec_argv, exec_envp) == -1 && temp != NULL)
    {
        memset(exec_path, '\0', sizeof(exec_path));
        temp = strtok(NULL, ":");
        strcpy(exec_path, temp);
        strcat(exec_path, "/strace");
    }
}

void parent(int pipe)
{
    char syscall_name[32], buf[4096], char_buf;
    double syscall_time = 0;
    int syscall_rec_cnt = 0, length = 0;

    now = time(NULL);
    printf("======sperf stat======\n");
    while (read(pipe, &char_buf, 1) > 0)
    {
        if (buf[0] == '+')
            break;
        buf[length++] = char_buf;
        if (buf[length - 1] == '\n' && buf[length - 2] == '>')
        {
            memset(syscall_name, '\0', sizeof(syscall_name));
            sscanf(buf, "%[^(]%*[^<]<%lf>\n", syscall_name, &syscall_time);
            memset(buf, '\0', sizeof(buf));
            length = 0;

            for (syscall_rec_cnt = 0; syscall_rec_cnt <= syscall_num; syscall_rec_cnt++)
            {
                if (strcmp(syscall_rec[syscall_rec_cnt].name, syscall_name) == 0)
                {
                    syscall_rec[syscall_rec_cnt].time += syscall_time;
                    total_exec_time += syscall_time;
                    break;
                }
            }
            if (syscall_rec_cnt > syscall_num)
            {
                syscall_num++;
                memcpy(syscall_rec[syscall_rec_cnt].name, syscall_name, sizeof(syscall_name));
                syscall_rec[syscall_rec_cnt].time = syscall_time;
                total_exec_time += syscall_time;
            }

            if (time(NULL) > now)
            {
                now++;
                for (int i = 0; i <= syscall_num; i++)
                    printf("%s(%.0lf%%)\n", syscall_rec[i].name, 100 * syscall_rec[i].time / total_exec_time);

                for (int i = 0; i < 80; i++)
                    printf("%c", '\0');
                printf("======================\n");
                fflush(stdout);
            }
        }
    }

    for (int i = 0; i <= (syscall_num > 5 ? 5 : syscall_num); i++)
        findtimemax();

    fflush(stdout);
}

int main(int argc, char *argv[], char *envp[])
{
    assert(argc >= 2);
    int channel[2];
    if (pipe(channel))
    {
        perror("Open Pipe Failed!");
        assert(0);
    }
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
