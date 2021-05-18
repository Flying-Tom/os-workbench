#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char func_template[] = "/home/flyingtom/os-workbench/crepl/tmp/crepl-XXXXXX";
int func_cnt = 0;
int main(int argc, char *argv[])
{
    static char line[4096];
    while (1)
    {
        printf("crepl> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin))
        {
            break;
        }
        mkstemp(func_template);
        printf("Got %zu chars.\n", strlen(line)); // ??
    }
}
