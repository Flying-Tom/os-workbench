#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <dirent.h>

int show_pids = 0, numeric_sort = 0, version = 0;

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

int main(int argc, char *argv[])
{
    ParameterMatch(argc, argv);
    printf("show-pids:%d\nnumeric-sort:%d\nversion:%d\n ", show_pids, numeric_sort, version);
    DIR *d;
    struct dirent *dir;
    d = opendir("/proc");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            printf("%s\n", dir->d_name);
        }
        closedir(d);
    }
    return 0;
}
