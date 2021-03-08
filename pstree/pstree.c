#include <stdio.h>
#include <assert.h>
#include <string.h>

bool show_pids = false, numeric_sort = false, version = false;

void ParameterMatch(int argc, char *argv[])
{

    for (int i = 0; i < argc; i++)
    {
        assert(argv[i]);
        if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--show-pids") == 0)
            show_pids = true;
        if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--numeric-sort") == 0)
            numeric_sort = true;
        if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version") == 0)
            version = true;
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
