# Memory debugger written in C for C

Use it to track memory leaks, wrong arguments to memory functions and writing past-block errors.

It's super lightweight (just 2 short files), but not super fast.

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

Add two files (*mem_debugger.c* & *mem_debugger.h*) from src folder to your project's src folder;

For each file you want to track allocations in, just include *mem_debugger.h* header in that file.

After that you're able to print all available info at current line using:

`mem_debugger_dump_info(int use_file, const char* file_name);`

To print to `stderr`, pass `(0, NULL)`;

To print to file, pass `(1, "file_name.txt)"`;

See *usage_example.c* for possible usage.

## Example output

```
Current unfreed allocations:
Source file                                Line  Mem. ptr   Bytes
src\usage_example.c                        12    00AC5588   10
src\usage_example.c                        12    00AC2558   9
src\usage_example.c                        12    00AC2518   8
src\usage_example.c                        12    00AC24D8   7
src\usage_example.c                        12    00AC2498   6

Unfreed total: 40 bytes from 5 allocation(s)

Top-5 heaviest allocations so far:
Source file                                Line  Bytes      Percentage
src\usage_example.c                        12    55         100.00%


Allocated total: 55 bytes
```
