#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>

#ifdef __x86_64__
#define ARCH "-m64"
#else
#define ARCH "-m32"
#endif

char src_path[256];
char so_path[256];
char *exec_argv[] = {
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
    sprintf(src_path, "/tmp/Crepl_src");
    sprintf(so_path, "/tmp/Crepl_%d.so", rand() % 100000);

    FILE *fp = fopen(src_path, "w");
    if (mode == FUNC)
        fprintf(fp, "%s", buf);
    else
        fprintf(fp, "int __expr_wrapper__(){ return (%s); }", buf);
    fclose(fp);

    pid_t pid = fork();
    if (pid == 0)
    {
        int trash = open("/dev/null", O_WRONLY);
        dup2(trash, STDOUT_FILENO);
        dup2(trash, STDERR_FILENO);
        execvp("gcc", exec_argv);
        assert(0);
    }
    else
    {
        int gcc_status = 0;
        wait(&gcc_status);
        if (WEXITSTATUS(gcc_status))
        {
            puts("Compile Error");
            //puts("\033[31m  Compile Error\033[0m");
        }
        else
        {
            if ((handle = dlopen(so_path, RTLD_LAZY | RTLD_GLOBAL)) != NULL)
                return true;
            else
            {
                puts("Load so Failed!");
                assert(0);
            }
        }
    }
    return false;
}

int main(int argc, char *argv[])
{
    static char line[4096];
    srand(time(NULL));
    while (1)
    {
        printf(">> ");
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin))
            break;
        else
        {
            line[strlen(line) - 1] = '\0';
            if (strncmp("int", line, 3) == 0)
            {
                if (Compile(line, FUNC))
                {
                    //printf("\033[32m  Added:\033[0m %s", line);
                    //puts("\033[32mOK.\033[0m");
                    puts("OK.");
                }
            }
            else
            {
                if (Compile(line, EXPR))
                {
                    int pid = fork();
                    if (pid == 0)
                    {
                        int (*wrapper_func)(void) = dlsym(handle, "__expr_wrapper__");
                        printf("  %s == %d.\n", line, wrapper_func());
                    }
                    else
                    {
                        wait(NULL);
                        dlclose(handle);
                    }
                    unlink(so_path);
                }
            }
        }
    }
}
