#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <sys/wait.h>
#include <time.h>

#ifdef __x86_64__
#define ARCH "-m64"
#else
#define ARCH "-m32"
#endif

char src_path[256] = {"/tmp/Crepl_src"};
char so_path[256];
char *exec_argv[] =
    {
        "gcc",
        "-x",
        "c",
        "-w",
        ARCH,
        src_path,
        "-shared",
        "-fPIC",
        "-o",
        so_path,
        NULL,
};

enum
{
    FUNC,
    EXPR,
};
void *handle = NULL;

bool Compile(char buf[], int mode)
{
    bool ret = false;
    char file_name[4096];

    srand(time(NULL));
    sprintf(so_path, "/tmp/Crepl_%d.so", rand());

    char wrapper[512];
    FILE *fp = fopen(src_path, "w");
    switch (mode)
    {
    case FUNC:
        fprintf(fp, "%s", buf);
        break;
    case EXPR:
        sprintf(wrapper, "int __expr_wrapper__(){ return (%s); }", buf);
        fprintf(fp, "%s", wrapper);
        break;
    default:
        break;
    }

    fclose(fp);

    pid_t pid = fork();
    if (pid == 0)
    {
        execvp("gcc", exec_argv);
        assert(0);
    }
    else
    {
        int gcc_status = 0;
        wait(&gcc_status);
        if (WEXITSTATUS(gcc_status))
            puts("\033[31m  Compile Error\033[0m");
        else
        {
            if ((handle = dlopen(so_path, RTLD_LAZY | RTLD_GLOBAL)) != NULL)
                ret = true;
            else
            {
                puts("Load so Failed!");
                assert(0);
            }
        }
    }
    return ret;
}

int main(int argc, char *argv[])
{
    static char line[4096];
    while (1)
    {
        printf("crepl> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin))
            break;
        else
        {
            if (strncmp("int", line, 3) == 0)
            {
                if (Compile(line, FUNC))
                    printf("\033[32m  Added:\033[0m %s", line);
            }
            else
            {
                line[strlen(line) - 1] = '\0';
                if (Compile(line, EXPR))
                {
                    int (*wrapper_func)(void) = dlsym(handle, "__expr_wrapper__");
                    printf(" %s = %d\n", line, wrapper_func());
                    dlclose(handle);
                }
            }
        }
    }
}
