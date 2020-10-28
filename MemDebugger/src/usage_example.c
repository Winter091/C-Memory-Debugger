#include "stdio.h"
#include "stdlib.h"
#include "time.h"


#include "mem_debugger.h"

int main()
{
    time_t start = clock();

    for (int i = 0; i < 1000000; i++)
    {
        int* a = malloc(10000);
        free(a);
    }

    time_t end = clock();
    printf("%d\n", end - start);
}
