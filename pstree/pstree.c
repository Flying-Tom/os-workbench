#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <dirent.h>

int show_pids = 0, numeric_sort = 0, version = 0;
char filename_buf[256];

int process_cnt = 0;

struct Process
{
    char name[64];
    __pid_t pid;
    __pid_t ppid;
} process[65536];

void ParameterMatch(int argc, char *argv[])
{

    for (int i = 0; i < argc; i++)
    {
        assert(argv[i]);
        if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--show-pids") == 0)
            show_pids = 1;
        if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--numeric-sort") == 0)
            numeric_sort = 1;
        if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0)
            version = 1;
        //printf("argv[%d] = %s\n", i, argv[i]);
    }
    assert(!argv[argc]);
};

void PrintVersion()
{
    puts("pstree (PSmisc) UNKNOWN");
    puts("Copyright (C) 2021 FlyingTom");
};

void BuildProcessTree(){

};

void PrintProcessTree(){

};

int main(int argc, char *argv[])
{
    ParameterMatch(argc, argv);
    //printf("show-pids:%d\nnumeric-sort:%d\nversion:%d\n", show_pids, numeric_sort, version);
    if (version)
    {
        PrintVersion();
        return 0;
    }
    DIR *d;
    struct dirent *dir;
    d = opendir("/proc");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            int pid = -1;
            //printf("%s\n", dir->d_name);
            sscanf(dir->d_name, "%d", &pid);
            if (pid != -1)
            {
                //printf("%d\n", pid);
                process[process_cnt].pid = pid;
                char stat_buf[512];
                sprintf(stat_buf, "/proc/%s/stat", dir->d_name);
                FILE *fp = fopen(stat_buf, "r");
                fscanf(fp, "%*s (%s) %*s %*s %d", process[process_cnt].name, &process[process_cnt].ppid);
                fclose(fp);
                process_cnt++;
            }
        }
        closedir(d);
    }
    for (int i = 0; i < 6; i++)
        printf("%s\n", process[i].name);
    return 0;
}
