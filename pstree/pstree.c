#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

int show_pids = 0, numeric_sort = 0, version = 0;
int process_cnt = 0;
int line_rec[32] = {};

char filename_buf[256], stat_buf[512];

__pid_t pidmap[65536] = {};
struct Process
{
    __pid_t pid;
    __pid_t ppid;
    char name[256];
    int children_num;
    struct Process *parent;
    struct Process *children[128];

} process[65536];

void NeedPrintVersion()
{
    if (version)
    {
        puts("pstree-lite_0.0.1_linux_x86_64");
        puts("Copyright (C) 2021 FlyingTom");
        exit(0);
    }
};

void NeedNumericSort()
{
    if (numeric_sort)
        return;
    for (int i = 2; i <= process_cnt; i++)
    {
        for (int j = i + 1; j <= process_cnt; j++)
        {
            if (strcmp(process[i].name + 1, process[j].name + 1) > 0)
            {
                struct Process process_temp = process[j];
                pidmap[process[j].pid] = i;
                pidmap[process[i].pid] = j;
                process[j] = process[i];
                process[i] = process_temp;
            }
        }
    }
};

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
    }
    assert(!argv[argc]);
};

void ProcessRead()
{
    struct dirent *dir;
    DIR *d = opendir("/proc");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            __pid_t pid = -1;
            sscanf(dir->d_name, "%d", &pid);
            if (pid != -1)
            {
                ++process_cnt;
                process[process_cnt].pid = pid;
                memset(stat_buf, '\0', sizeof(stat_buf));
                sprintf(stat_buf, "/proc/%s/stat", dir->d_name);
                FILE *fp = fopen(stat_buf, "r");
                if (fp)
                {
                    fscanf(fp, "%*d %s %*s %d", process[process_cnt].name, &process[process_cnt].ppid);
                    fclose(fp);
                }
                else
                    exit(2);

                process[process_cnt].name[strlen(process[process_cnt].name) - 1] = '\0';
                process[process_cnt].name[0] = '\0';
                pidmap[pid] = process_cnt;
            }
        }
        closedir(d);
    }
    else
        exit(2);
};

void BuildProcessTree()
{
    for (int i = 1; i < process_cnt; i++)
        process[pidmap[process[i].ppid]].children[process[pidmap[process[i].ppid]].children_num++] = &process[i];
};

void PrintProcessTree(struct Process *cur, int deepth)
{
    char line_temp;
    for (int i = 0; i < deepth - 1; i++)
    {
        line_temp = (line_rec[i]) ? '|' : ' ';
        printf("%c       ", line_temp);
    }

    if (deepth)
        printf("+-------");

    if (show_pids)
        printf("%s(%d)\n", cur->name + 1, cur->pid);
    else
        printf("%s\n", cur->name + 1);

    line_rec[deepth] = 1;
    for (int i = 0; i < cur->children_num; i++)
    {
        for (int i = 0; i <= deepth; i++)
        {
            line_temp = (line_rec[i]) ? '|' : ' ';
            printf("%c%7s", line_temp);
        }
        printf("\n");
        if (i + 1 == cur->children_num)
            line_rec[deepth] = 0;
        PrintProcessTree(cur->children[i], deepth + 1);
    }
};

int main(int argc, char *argv[])
{
    ParameterMatch(argc, argv);
    NeedPrintVersion();
    ProcessRead();
    NeedNumericSort();
    BuildProcessTree();
    PrintProcessTree(&process[1], 0);
    return 0;
}
