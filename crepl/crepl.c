#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <sys/wait.h>

char func_template[] = "/home/flyingtom/os-workbench/crepl/tmp/creplXXXXXX";
int func_cnt = 0;
int expr_cnt = 0;
//char src_path[] = {"/home/flyingtom/os-workbench/crepl/tmp/creplsrc.c"};
char src_path[256];
char so_path[256];
char *exec_argv[] =
    {
        "gcc",
        "-x",
        "c",
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
            return false;
        }
        else
        {
            puts("\033[32m  OK\033[0m");
            return true;
        }
    }
    return false;
}

void FuncBuild(char buf[])
{
    FILE *fp = fopen(src_path, "w");
    fprintf(fp, "%s", buf);
    fclose(fp);
    //printf("tmp_path:%s\n", tmp_path);
    printf("gcc %s -shared -fPIC -o %s\n", src_path, so_path);

    if (Compile())
    {
        void *handle = NULL;
        if ((handle = dlopen(so_path, RTLD_NOW)) != NULL)
        {
        }
    }
}

void ExprCal(char buf[])
{
    char wrapper[512];
    sprintf(wrapper, "int __expr_wrapper_%d(){ return %s;}", expr_cnt, buf);
    if ((handle = dlopen(so_path, RTLD_NOW)) != NULL)
    {
    }
}

int main(int argc, char *argv[])
{
    static char line[4096];
    char template_src[] = {"/home/flyingtom/os-workbench/crepl/tmp/Crepl_SRCXXXXXX"};
    char template_so[] = {"/home/flyingtom/os-workbench/crepl/tmp/Crepl_SOXXXXXX"};
    int fd_src = mkstemp(template_src);
    int fd_so = mkstemp(template_so);
    sprintf(line, "/proc/self/fd/%d", fd_src);
    readlink(line, src_path, sizeof(src_path) - 1);
    sprintf(line, "/proc/self/fd/%d", fd_so);
    readlink(line, so_path, sizeof(src_path) - 1);
    //printf("%s\n", src_path);
    //printf("%s\n", so_path);
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
            //printf("Got %zu chars.\n", strlen(line));
            memset(line, '\0', sizeof(line));
        }
    }
}
