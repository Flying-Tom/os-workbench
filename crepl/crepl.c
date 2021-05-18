#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char func_template[] = "/home/flyingtom/os-workbench/crepl/tmp/crepl-XXXXXX";
int func_cnt = 0;
int main(int argc, char *argv[])
{
    static char line[4096];
    for (int i = 1; i <= 5; i++)
        mkstemp(func_template);

    while (1)
    {
        printf("crepl> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin))
        {
            break;
        }
        printf("Got %zu chars.\n", strlen(line)); // ??
    }
}
