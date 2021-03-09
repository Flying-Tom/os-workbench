#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <dirent.h>

int show_pids = 0, numeric_sort = 0, version = 0;
char filename_buf[256];

int process_cnt = 0;

__pid_t pidmap[65536] = {};
struct Process
{
    char name[128];
    __pid_t pid;
    __pid_t ppid;
    int children_num;
    struct Process *parent;
    struct Process *children[128];

} process[4096];

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

void ProcessRead()
{
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
                //printf("%d %d\n", pid, process_cnt);
                ++process_cnt;
                process[process_cnt].pid = pid;
                char stat_buf[512];
                sprintf(stat_buf, "/proc/%s/stat", dir->d_name);
                FILE *fp = fopen(stat_buf, "r");
                fscanf(fp, "%*d %s %*s %d", process[process_cnt].name, &process[process_cnt].ppid);
                fclose(fp);
                process[process_cnt].name[strlen(process[process_cnt].name) - 1] = '\0';
                process[process_cnt].name[0] = '\0';
                pidmap[pid] = process_cnt;
            }
        }
        closedir(d);
    }
};

void BuildProcessTree()
{
    for (int i = 1; i < process_cnt; i++)
    {
        //printf("children_num:%d\n", process[process[i].ppid].children_num);
        process[pidmap[process[i].ppid]].children[process[pidmap[process[i].ppid]].children_num++] = &process[i];
        //printf("pid:%d ppid:%d\n", process[i].pid, process[i].ppid);
    }
};

int line_rec[16] = {};

void PrintProcessTree(struct Process *cur, int deepth)
{
    for (int i = 0; i < deepth; i++)
    {
        if (line_rec[0])
            printf("|");
        else
            printf(" ");
        printf("       ");
    }

    if (deepth)
        printf("+-------");
    if (show_pids)
        printf("%s(%d)\n", cur->name + 1, cur->pid);
    else
        printf("%s\n", cur->name + 1);

    for (int i = 0; i < cur->children_num; i++)
    {
        printf("\n");
        line_rec[deepth] = 1;
        PrintProcessTree(cur->children[i], deepth + 1);
    }
};

int main(int argc, char *argv[])
{
    ParameterMatch(argc, argv);
    if (version)
    {
        PrintVersion();
        return 0;
    }
    ProcessRead();
    BuildProcessTree();
    PrintProcessTree(&process[1], 0);
    return 0;
}
