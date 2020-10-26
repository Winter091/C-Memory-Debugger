#ifndef MEM_DEBUGGER_H_
#define MEM_DEBUGGER_H_

#include "stdlib.h"
#include "stdint.h"

void* debug_malloc (size_t size,                const char* file_name, const char* func_name, int src_line);
void* debug_calloc (size_t num, size_t size,    const char* file_name, const char* func_name, int src_line);
void* debug_realloc(void* ptr, size_t new_size, const char* file_name, const char* func_name, int src_line);
void  debug_free   (void* ptr,                  const char* file_name, const char* func_name, int src_line);

#ifndef MEM_DEBUG_DISABLE
#define malloc(size)           debug_malloc (size,          __FILE__, __PRETTY_FUNCTION__ , __LINE__)
#define calloc(num, size)      debug_calloc (num, size,     __FILE__, __PRETTY_FUNCTION__ , __LINE__)
#define realloc(ptr, new_size) debug_realloc(ptr, new_size, __FILE__, __PRETTY_FUNCTION__ , __LINE__)
#define free(ptr)              debug_free   (ptr,           __FILE__, __PRETTY_FUNCTION__ , __LINE__)
#endif

void print_allocations_file(const char* file_path);
void print_allocations_console();

#endif
