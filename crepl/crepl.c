#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char func_template[] = "/home/flyingtom/os-workbench/crepl/tmp/crepl-XXXXXX";
int func_cnt = 0;
int fd;
char file_path[128] = {'\0'}, tmp_path[128] = {'\0'};

void FuncBuild(char buf[])
{
    FILE *fp = fopen(tmp_path, "a+");
    fprintf(fp, "%s", buf);
    fclose(fp);

    char *exec_argv[] =
        {"gcc",
         tmp_path,
         "-shared",
         "-fPIC",
         "-o",
         strcat(tmp_path, ".so")};

    if (execvp("gcc", exec_argv) == -1)
    {
        printf("Compile Error\n");
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
    printf("%s\n", tmp_path);
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
