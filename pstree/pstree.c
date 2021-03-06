#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdbool.h>

#define TABSIZE 2

int process_cnt = 0;
int line_rec[32] = {};
int last_child_flag = 0;

char filename_buf[256], stat_buf[512];

__pid_t pidmap[65536] = {}, pidarg = 1;
struct Process
{
    __pid_t pid;
    __pid_t ppid;
    char name[256];
    int children_cnt;
    struct Process *children[128];

} process[65536];

bool NeedPrintVersion(int version)
{
    if (version)
    {
        fprintf(stderr, "pstree-lite_v0.0.1_linux_x86_64\n");
        fprintf(stderr, "Copyright (C) 2021 FlyingTom\n");
        return true;
    }
    return false;
};

void NeedNumericSort(int numeric_sort)
{
    if (numeric_sort)
        return;
    for (int i = 2; i <= process_cnt; i++)
    {
        for (int j = i + 1; j <= process_cnt; j++)
        {
            if (strcmp(process[i].name, process[j].name) > 0)
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

void ParameterMatch(int argc, char *argv[], int *show_pids, int *numeric_sort, int *version)
{

    for (int i = 0; i < argc; i++)
    {
        assert(argv[i]);
        if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--show-pids") == 0)
            *show_pids = 1;
        if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--numeric-sort") == 0)
            *numeric_sort = 1;
        if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0)
            *version = 1;
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
                    memset(filename_buf, '\0', sizeof(filename_buf));
                    fscanf(fp, "%*d %s %*s %d", filename_buf, &process[process_cnt].ppid);
                    fclose(fp);
                    strcpy(process[process_cnt].name, filename_buf + 1);
                    process[process_cnt].name[strlen(process[process_cnt].name) - 1] = '\0';
                    pidmap[pid] = process_cnt;
                }
                else
                    exit(2);
            }
        }
        closedir(d);
    }
    else
        exit(2);
};

void BuildProcessTree()
{
    for (int i = 1; i <= process_cnt; i++)
        process[pidmap[process[i].ppid]].children[process[pidmap[process[i].ppid]].children_cnt++] = &process[i];
};

void PrintProcessTree(struct Process *cur, int deepth, int show_pids)
{
    char line_temp;
    for (int i = 0; i < deepth - 1; i++)
    {
        line_temp = (line_rec[i]) ? '|' : ' ';
        printf("%c%2s", line_temp, "");
    }

    if (deepth)
    {
        if (last_child_flag)
            printf("??????");
        else
            printf("??????");
        last_child_flag = 0;
    }

    if (show_pids)
        printf("%s(%d)\n", cur->name, cur->pid);
    else
        printf("%s\n", cur->name);

    line_rec[deepth] = 1;
    for (int i = 0; i < cur->children_cnt; i++)
    {
        if (i + 1 == cur->children_cnt)
        {
            line_rec[deepth] = 0;
            last_child_flag = 1;
        }
        PrintProcessTree(cur->children[i], deepth + 1, show_pids);
    }
};

/*
void PrintProcessTree(struct Process *cur, int deepth, int show_pids)
{
    for (int i = 0; i < deepth; i++)
        printf("%4s", "");

    if (show_pids)
        printf("%s(%d)\n", cur->name, cur->pid);
    else
        printf("%s\n", cur->name);

    for (int i = 0; i < cur->children_cnt; i++)
        PrintProcessTree(cur->children[i], deepth + 1, show_pids);
};
*/
int main(int argc, char *argv[])
{
    int show_pids = 0, numeric_sort = 0, version = 0;
    ParameterMatch(argc, argv, &show_pids, &numeric_sort, &version);
    if (NeedPrintVersion(version))
        return 0;
    ProcessRead();
    NeedNumericSort(numeric_sort);
    BuildProcessTree();
    PrintProcessTree(&process[pidmap[pidarg]], 0, show_pids);
    return 0;
}
