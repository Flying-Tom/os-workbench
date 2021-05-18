#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char func_template[] = "/home/flyingtom/os-workbench/crepl/tmp/creplXXXXXX";
int func_cnt = 0;
int fd;
char file_path[128] = {'\0'}, tmp_path[128] = {'\0'}, so_path[128] = {'\0'};

void FuncBuild(char buf[])
{
    FILE *fp = fopen(tmp_path, "a+");
    fprintf(fp, "%s", buf);
    fclose(fp);

    strcpy(so_path, tmp_path);
    strcat(so_path, ".so");
    printf("tmp_path:%s\n", tmp_path);
    char *exec_argv[] =
        {
            "gcc",
            tmp_path,
            "-shared",
            "-fPIC",
            "-o",
            so_path,
        };
    printf("gcc %s -shared -fPIC -o %s\n", tmp_path, so_path);

    if (execvp("gcc", exec_argv) == -1)
    {
        puts("\033[31mCompile Error\033[0m");
    }
    else
    {
    }
}

void ExprCal(char buf[])
{
}

int main(int argc, char *argv[])
{
    static char line[4096];
    fd = mkstemp(func_template);
    sprintf(line, "/proc/self/fd/%d", fd);
    readlink(line, file_path, sizeof(file_path) - 1);
    strcpy(tmp_path, strcat(file_path, "tmp"));
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
