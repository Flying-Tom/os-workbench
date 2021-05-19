#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <dlfcn.h>

char func_template[] = "/home/flyingtom/os-workbench/crepl/tmp/creplXXXXXX";
int func_cnt = 0;
int fd;
char file_path[] = {"/home/flyingtom/os-workbench/crepl/tmp/creplsrc.c"};
char tmp_path[] = {"/home/flyingtom/os-workbench/crepl/tmp/crepltmp.c"};
char so_path[] = {"/home/flyingtom/os-workbench/crepl/tmp/crepl.so"};
char *exec_argv[] =
    {
        "gcc",
        tmp_path,
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
        dup2(compile_pipe[1], STDOUT_FILENO);
        execvp("gcc", exec_argv);
    }
    else
    {
        char buf[512];
        if (read(compile_pipe[0], buf, 512) > 0)
        {
            puts("\033[31mCompile Error\033[0m");
            int cp_pid = fork();
            if (cp_pid == 0)
            {
                char *cp_argv[] =
                    {
                        "cp",
                        file_path,
                        tmp_path,
                        NULL,
                    };
                execvp("cp", cp_argv);
            }
        }
        else
        {
            puts("\033[32mOK\033[0m");
            FILE *fp = fopen(file_path, "a+");
            fprintf(fp, "%s", buf);
        }
    }
}

void FuncBuild(char buf[])
{
    FILE *fp = fopen(tmp_path, "a+");
    fprintf(fp, "%s", buf);
    fclose(fp);

    printf("tmp_path:%s\n", tmp_path);
    printf("gcc %s -shared -fPIC -o %s\n", tmp_path, so_path);

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
    readlink(line, file_path, sizeof(file_path) - 1);
    strcpy(tmp_path, strcat(file_path, "tmp"));
    */
    unlink(file_path);
    unlink(tmp_path);
    unlink(so_path);
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
            printf("Got %zu chars.\n", strlen(line));
            memset(line, '\0', sizeof(line));
        }
    }
}
