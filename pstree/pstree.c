#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <dirent.h>

int show_pids = 0, numeric_sort = 0, version = 0;
char filename_buf[256];

int process_cnt = 0;

struct Process
{
    char name[128];
    __pid_t pid;
    __pid_t ppid;
    int children_num;
    struct Process *parent;
    struct Process *children[128];

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

void BuildProcessTree()
{
    for (int i = 0; i < process_cnt; i++)
    {
        *process[process[i].ppid].children[process[process[i].ppid].children_num++] = process[i];
    }
};

void PrintProcessTree(struct Process *cur, int deepth)
{
    for (int i = 0; i < deepth; i++)
        printf("\t");
    printf("%s\n", cur->name);
    for (int i = 0; i < cur->children_num; i++)
    {
        PrintProcessTree(cur->children[i], deepth + 1);
    }
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
            __pid_t pid = -1;
            sscanf(dir->d_name, "%d", &pid);
            if (pid != -1)
            {
                process[process_cnt].pid = pid;
                char stat_buf[512];
                sprintf(stat_buf, "/proc/%s/stat", dir->d_name);
                FILE *fp = fopen(stat_buf, "r");
                fscanf(fp, "%*s (%s) %*s %*s %d", process[process_cnt].name, &process[process_cnt].ppid);
                fclose(fp);
                process[process_cnt].name[strlen(process[process_cnt].name) - 1] = '\0';
                process_cnt++;
            }
        }
        process_cnt--;
        closedir(d);
    }
    puts("Preprocess Completed!");
    //for (int i = 0; i < 200; i++)
    //   printf("%d:%s\n", process[i].pid, process[i].name);
    BuildProcessTree();
    puts("Buildtree Completed!");
    PrintProcessTree(&process[1], 0);
    puts("Printtree Completed!");
    return 0;
}
