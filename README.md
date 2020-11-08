# Memory debugger written in C for C

Use it to track memory leaks, wrong arguments to memory functions, writing to past-block memory 
and get info about heaviest allocations.

It's super lightweight (just 2 short files), but definitely not fast and memory-efficient.

## Features

### 1. Tracking memory leaks

At any time you're able to print all allocated, but yet not deallocated blocks of memory using 

`mem_debugger_dump_info(int use_file, const char* file_name);`

It's useful to find forgotten deallocations!

### 2. Tracking wrong arguments to memory functions

If you, for example, free the same pointer 2 times in a row, or pass a pointer to realloc

which is not pointing to active heap memory, the warning will be printed and abort() will be called
immediately.

### 3. Writing-past-block checking

Example of this type of error: 

```
int* a = malloc(3 * sizeof(int));

// writing past allocated memory
a[3] = 0;
```

The debugger, when allocating, writes additional bytes at the end of the memory block, 

and during `free()` function checks whether the data after block has changed or not.

If the data is corrupted, the warning will be printed and and abort() will be called
immediately.

Note: it's impossible to check if writing past-bound has occurred if the pointer to

that memory was never deallocated, i.e freed. This theck only happens in `free()`.

### 4. Tracking heavy allocations

 When `mem_debugger_dump_info()` is called, the debugger also prints up to 5 heaviest 
 
 (largest) allocation sites. Allocation size is the particular line in source code, so
 
 ```
 for (int i = 0; i < 10; i++)
     malloc(100);
 ```
 
 is treated as one allocation site.

## Usage

Add *mem_debugger.h* to your project's src folder;

Include `mem_debugger.h` in all source files which contain memory (de)allocation functions;

In **one** of the files, define `MEM_DEBUGGER_IMPLEMENTATION` just before including `mem_debugger.h`, like that:

```
#define MEM_DEBUGGER_IMPLEMENTATION
#include "mem_debugger.h"
```

After that you're able to print all available info at current line using:

`mem_debugger_dump_info(int use_file, const char* file_name);`

To print to `stderr`, pass `(0, NULL)`;

To print to file, pass `(1, "file_name.txt)"`;

See *usage_example.c* for possible usage.

## Example output

```
Current unfreed allocations:
Source file                                Line  Bytes
MemDebugger/src/usage_example.c            15    10
MemDebugger/src/usage_example.c            15    9
MemDebugger/src/usage_example.c            15    8
MemDebugger/src/usage_example.c            15    7
MemDebugger/src/usage_example.c            15    6

Unfreed total: 40 bytes from 5 allocation(s)

Top-5 heaviest allocations so far:
Source file                                Line  Bytes      Percentage
MemDebugger/src/usage_example.c            15    55         100.00%

Allocated total: 55 bytes
```
