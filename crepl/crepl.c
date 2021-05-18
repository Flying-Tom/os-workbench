#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>

char func_template[] = "/home/flyingtom/os-workbench/crepl/tmp/creplXXXXXX";
int func_cnt = 0;
int fd;
char file_path[] = {"/home/flyingtom/os-workbench/crepl/tmp/creplsrc.c"};
char tmp_path[] = {"/home/flyingtom/os-workbench/crepl/tmp/crepltmp.c"};
char so_path[] = {"/home/flyingtom/os-workbench/crepl/tmp/crepl.so"};

void FuncBuild(char buf[])
{
    FILE *fp = fopen(tmp_path, "a+");
    fprintf(fp, "%s", buf);
    fclose(fp);

    printf("tmp_path:%s\n", tmp_path);
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
    printf("gcc %s -shared -fPIC -o %s\n", tmp_path, so_path);

    if (execvp("gcc", exec_argv) == -1)
    {
        puts("\033[31mCompile Error\033[0m");
        execvp("cp", (char *){"cp", file_path, tmp_path});
    }
    else
    {
        fprintf(fp, "%s", buf);
    }
}

void ExprCal(char buf[])
{
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
