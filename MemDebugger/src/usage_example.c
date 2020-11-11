#include "stdio.h"
#include "stdlib.h"

// include this header in all files you want
// to debug memory allocations in;
// use that define strictly! in one source file
// to create implementation for debugger.
#define MEM_DEBUGGER_IMPLEMENTATION
#include "mem_debugger.h"

int main()
{
    void* ptrs[10];

    for (int i = 0; i < 10; i++)
        ptrs[i] = malloc(i + 1);

    // whoops! 5 instead of 10
    for (int i = 0; i < 5; i++)
        free(ptrs[i]);

    // print data to stderr (console)
    mem_debugger_dump_info(0, NULL);

    // ...or print to file
    mem_debugger_dump_info(1, "alloc_data.txt");
}
