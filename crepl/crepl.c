#include <stdio.h>
#include <string.h>

//char func_template[] = "crepl-XXXXXX";
char func_template[];
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
        printf("Got %zu chars.\n", strlen(line)); // ??
    }
}
