# Memory debugger written in C for C

It is used to track memory allocations and, most importantly, forgotten memory deallocations.

It's super lightweight (just 2 short files), but not super fast.

## Usage

Add two files (*mem_debugger.c* & *mem_debugger.h*) from src folder to your project's src folder;

For each file you want to track allocations in, just type:

`#include "mem_debugger.h"` before any function begins.

After that you're able to print all unfreed allocations at current line using:

`print_allocations_console();` or

`print_allocations_file("Allocations.txt");` (Parameter is file name).

See *usage_example.c* for possible usage.
