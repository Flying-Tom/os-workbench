#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char func_template[] = "/home/flyingtom/os-workbench/crepl/tmp/crepl-XXXXXX";
int func_cnt = 0;
int fd;
char file_path[128] = {'\0'};

void FuncBuild(char buf[])
{
    /*
    fd = mkstemp(func_template);
    printf("int func() fd:%d\n", fd);
    printf("fd:%d buf:%s \n", fd, buf);
    write(fd, buf, strlen(buf));
    close(fd);
    */
}

int main(int argc, char *argv[])
{
    static char line[4096];
    char buf[1024] = {'\0'};
    fd = mkstemp(func_template);
    snprintf(buf, sizeof(buf), "/proc/self/fd/%d", fd);
    readlink(buf, file_path, sizeof(file_path) - 1);
    printf("%s\n", file_path);
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
