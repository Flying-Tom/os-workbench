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
void *handle = NULL;

bool Compile()
{
    bool ret = false;
    char buf[4096];
    int fd_src = mkstemp(template_src);
    int fd_so = mkstemp(template_so);
    sprintf(buf, "/proc/self/fd/%d", fd_src);
    readlink(buf, src_path, sizeof(src_path) - 1);
    sprintf(buf, "/proc/self/fd/%d", fd_so);
    readlink(buf, so_path, sizeof(src_path) - 1);
    int gcc_status = 0;
    int pid = fork();
    if (pid == 0)
    {
        execvp("gcc", exec_argv);
    }
    else
    {
        wait(&gcc_status);
        if (WEXITSTATUS(gcc_status))
        {
            puts("\033[31m  Compile Error\033[0m");
        }
        else
        {
            if ((handle = dlopen(so_path, RTLD_LAZY | RTLD_GLOBAL)) != NULL)
                ret = true;
        }
    }
    return ret;
}

void FuncBuild(char buf[])
{
    FILE *fp = fopen(src_path, "w");
    fprintf(fp, "%s", buf);
    fclose(fp);

    if (Compile())
    {
        printf("\033[32m  Added:\033[0m %s", buf);
    }
}

void ExprCal(char buf[])
{
    char wrapper[512];
    buf[strlen(buf) - 1] = '\0';
    sprintf(wrapper, "int __expr_wrapper__(){ return (%s); }", buf);
    FILE *fp = fopen(src_path, "w");
    fprintf(fp, "%s", wrapper);
    fclose(fp);
    if (Compile())
    {
        int (*func)(void) = dlsym(handle, "__expr_wrapper__");
        printf(" %s = %d\n", buf, func());
        dlclose(handle);
    }
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
                FuncBuild(line);
            else
                ExprCal(line);
            //memset(line, '\0', sizeof(line));
        }
    }
}
