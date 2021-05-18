#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char func_template[] = "/home/flyingtom/os-workbench/crepl/tmp/crepl-XXXXXX";
int func_cnt = 0;
int fd;
FILE *fp;

void FuncBuild(char buf[])
{
    fp = fdopen(fd, "a+");
    printf("int func() fd:%d\n", fd);
    fprintf(fp, "%s\n", buf);
    fclose(fp);
}

int main(int argc, char *argv[])
{
    static char line[4096];
    fd = mkstemp(func_template);
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
                printf("expr\n");
            printf("Got %zu chars.\n", strlen(line));
            memset(line, '\0', sizeof(line));
        }
    }
}
