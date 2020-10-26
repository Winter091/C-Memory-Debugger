#include "stdio.h"
#include "stdlib.h"

// just include this header in every source file
// you want to track
#include "mem_debugger.h"

int main()
{
    int* a = malloc(6);

    // print allocations info at every moment you want!

    // this will print info containing one allocation
    print_allocations_file("Allocations.txt");

    int* b = calloc(2, 1);
    int* c = malloc(22);

    // this will print info containing three allocations
    print_allocations_console();

    free(a);
    free(b);
    free(c);

    // this will print info containing zero allocations
    print_allocations_console();
}
