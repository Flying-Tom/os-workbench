#include <stdio.h>
#include <assert.h>
#include <string.h>

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
    printf("show-pids:%d\n numeric-sort:%d\n version:%d\n ", show_pids, numeric_sort, version);
    return 0;
}
