#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <sys/wait.h>

char src_path[256], so_path[256];
char template_src[] = {"/tmp/Crepl_SRCXXXXXX"};
char template_so[] = {"/tmp/Crepl_SOXXXXXX"};
char *exec_argv[] =
    {
        "gcc",
        "-x",
        "c",
        "-w",
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
int expr_cnt = 0;
void *handle = NULL;

bool Compile(char buf[], int mode)
{
    bool ret = false;
    char file_name[4096];
    int fd_src = mkstemp(template_src);
    int fd_so = mkstemp(template_so);
    sprintf(file_name, "/proc/self/fd/%d", fd_src);
    readlink(file_name, src_path, sizeof(src_path) - 1);
    sprintf(file_name, "/proc/self/fd/%d", fd_so);
    readlink(file_name, so_path, sizeof(src_path) - 1);

    FILE *fp = fopen(src_path, "w");
    if (mode == FUNC)
    {
        fprintf(fp, "%s", buf);
    }
    else
    {
        char wrapper[512];
        sprintf(wrapper, "int __expr_wrapper__%d(){ return (%s); }", expr_cnt, buf);
        fprintf(fp, "%s", wrapper);
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
                {
                    printf("\033[32m  Added:\033[0m %s", line);
                }
            }
            else
            {
                line[strlen(line) - 1] = '\0';
                if (Compile(line, EXPR))
                {
                    char wrapper_name[512];
                    sprintf(wrapper_name, "__expr_wrapper__%d", expr_cnt++);
                    printf("%s\n", wrapper_name);
                    int (*func)(void) = dlsym(handle, wrapper_name);
                    printf(" %s = %d\n", line, func());
                }
            }
            //memset(line, '\0', sizeof(line));
        }
    }
}
