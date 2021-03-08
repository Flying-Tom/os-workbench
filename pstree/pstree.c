#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <dirent.h>

int show_pids = 0, numeric_sort = 0, version = 0;
char filename_buf[256];
int pid_rec[65536] = {}, pid_cnt = 0;
int ppid[65536] = {};

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
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    assert(!argv[argc]);
};

void BuildProcessTree(){

};

void PrintProcessTree(){

};

int main(int argc, char *argv[])
{
    ParameterMatch(argc, argv);
    printf("show-pids:%d\nnumeric-sort:%d\nversion:%d\n", show_pids, numeric_sort, version);
    DIR *d;
    struct dirent *dir;
    d = opendir("/proc");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            int PID = -1;
            //printf("%s\n", dir->d_name);
            sscanf(dir->d_name, "%d", &PID);
            if (PID != -1)
            {
                printf("%d\n", PID);
                char stat_buf[512];
                sprintf(stat_buf, "/proc/%s/stat", dir->d_name);
                printf("%s\n", stat_buf);
                FILE *fp = fopen(stat_buf, "r");
                fscanf(fp, "%*s %*s %*s %*s %d", &ppid[PID]);
                printf("ppid[%d]:%d\n", PID, ppid[PID]);
                pid_rec[pid_cnt++] = PID;
            }
        }
        closedir(d);
    }
    return 0;
}
