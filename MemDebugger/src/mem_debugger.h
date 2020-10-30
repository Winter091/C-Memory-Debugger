#ifndef MEM_DEBUGGER_H_
#define MEM_DEBUGGER_H_

#include "stdlib.h"

void* debug_malloc (size_t size,                const char* file_name, int src_line);
void* debug_calloc (size_t num, size_t size,    const char* file_name, int src_line);
void* debug_realloc(void* ptr, size_t new_size, const char* file_name, int src_line);
void  debug_free   (void* ptr,                  const char* file_name, int src_line);

void mem_debugger_dump_info(int use_file, const char* file_name);

#ifndef MEM_DEBUG_DISABLE
#define malloc(size)           debug_malloc (size,          __FILE__, __LINE__)
#define calloc(num, size)      debug_calloc (num, size,     __FILE__, __LINE__)
#define realloc(ptr, new_size) debug_realloc(ptr, new_size, __FILE__, __LINE__)
#define free(ptr)              debug_free   (ptr,           __FILE__, __LINE__)
#endif

#endif
