#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <sys/wait.h>

#ifdef __x86_64__
#define ARCH "-m64"
#else
#define ARCH "-m32"
#endif

char src_path[256], so_path[256];
char template_src[] = {"/home/flyingtom/os-workbench/crepl/tmp/Crepl_SRCXXXXXX"};
char template_so[] = {"/home/flyingtom/os-workbench/crepl/tmp/Crepl_SOXXXXXX"};
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
int (*wrapper_func)(void);

bool Compile(char buf[], int mode)
{
    bool ret = false;
    char file_name[4096];
    unlink(template_src);
    unlink(template_so);
    int fd_src = mkstemp(template_src);
    int fd_so = mkstemp(template_so);
    sprintf(file_name, "/proc/self/fd/%d", fd_src);
    readlink(file_name, src_path, sizeof(src_path) - 1);
    sprintf(file_name, "/proc/self/fd/%d", fd_so);
    readlink(file_name, so_path, sizeof(src_path) - 1);

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
    int gcc_status = 0;
    if (pid == 0)
    {
        execvp("gcc", exec_argv);
    }
    else
    {
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
            if (mode == EXPR)
            {
                wrapper_func = NULL;
                wrapper_func = dlsym(handle, "__expr_wrapper__");
                fork();
                if (pid == 0)
                {
                    printf(" %s = %d\n", buf, wrapper_func());
                    exit(0);
                }
                else
                {
                    wait(NULL);
                    dlclose(handle);
                }

                assert(wrapper_func);
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
                Compile(line, EXPR);
            }
        }
    }
}
