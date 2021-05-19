#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <dlfcn.h>

char func_template[] = "/home/flyingtom/os-workbench/crepl/tmp/creplXXXXXX";
int func_cnt = 0;
int fd;
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

void Compile()
{
    int pid = fork();
    int compile_pipe[2];
    pipe(compile_pipe);
    if (pid == 0)
    {
        close(compile_pipe[0]);
        dup2(compile_pipe[1], STDERR_FILENO);
        execvp("gcc", exec_argv);
    }
    else
    {
        close(compile_pipe[1]);
        char buf[512];
        read(compile_pipe[0], buf, 256);
        printf("%s\n", buf);
        if (strlen(buf) > 1)
        {
            puts("\033[31mCompile Error\033[0m");
        }
        else
        {
            puts("\033[32mOK\033[0m");
            FILE *fp = fopen(src_path, "a+");
            //fprintf(fp, "%s", buf);
        }
    }
}

void FuncBuild(char buf[])
{
    FILE *fp = fopen(src_path, "a+");
    fprintf(fp, "%s", buf);
    fclose(fp);
    //printf("tmp_path:%s\n", tmp_path);
    printf("gcc %s -shared -fPIC -o %s\n", src_path, so_path);

    Compile();
}

void ExprCal(char buf[])
{
    void *handle = NULL;
    if ((handle = dlopen(so_path, RTLD_NOW)) != NULL)
    {
    }
}

int main(int argc, char *argv[])
{
    static char line[4096];
    /*
    fd = mkstemp(func_template);
    sprintf(line, "/proc/self/fd/%d", fd);
    readlink(line, src_path, sizeof(src_path) - 1);
    strcpy(tmp_path, strcat(src_path, "tmp"));
    */
    char template_src[] = {"/home/flyingtom/os-workbench/crepl/tmp/Crepl_SRCXXXXXX"};
    char template_so[] = {"/home/flyingtom/os-workbench/crepl/tmp/Crepl_SOXXXXXX"};
    int fd_src = mkstemp(template_src);
    int fd_so = mkstemp(template_so);
    sprintf(line, "/proc/self/fd/%d", fd_src);
    readlink(line, src_path, sizeof(src_path) - 1);
    sprintf(line, "/proc/self/fd/%d", fd_so);
    readlink(line, so_path, sizeof(src_path) - 1);
    printf("%s\n", src_path);
    printf("%s\n", so_path);
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
