#include "stdio.h"

#define MEM_DEBUG_DISABLE
#include "mem_debugger.h"

// ================ Struct definitions ===================
typedef struct
{
    void* block;
    size_t size;
    const char* file_name;
    const char* func_name;
    uint32_t src_line;
}
MemAllocData;

typedef struct MemAllocsNode
{
    MemAllocData* data;
    struct MemAllocsNode* ptr_next;
}
MemAllocsNode;

typedef struct
{
    MemAllocsNode *head, *tail;
}
MemAllocsList;

// ================ Struct definitions end ================

// ============ Linked list implementation ================

MemAllocsList* list_create()
{
    MemAllocsList* list = malloc(sizeof(MemAllocsList));
    list->head = NULL;
    list->tail = NULL;
    return list;
}

void list_push_back(MemAllocsList* list, MemAllocData* elem)
{
    MemAllocsNode* node = malloc(sizeof(MemAllocsNode));
    node->data = elem;
    node->ptr_next = NULL;

    // size = 0
    if (!list->head)
    {
        list->head = node;
        list->tail = node;
    }

    else
    {
        MemAllocsNode* prev_tail = list->tail;
        list->tail = node;
        prev_tail->ptr_next = node;
    }
}

void list_remove(MemAllocsList* list, MemAllocData* elem)
{
    // size == 0
    if (!list->head) return;

    // size == 1
    if (list->head == list->tail)
    {
        if (list->head->data->block == elem->block)
        {
            free(list->head->data);
            free(list->head);
            list->head = NULL;
            list->tail = NULL;
        }
    }

    // size > 1
    else
    {
        MemAllocsNode* curr_node = list->head;
        while (curr_node && curr_node->data->block != elem->block)
            curr_node = curr_node->ptr_next;

        if (!curr_node)
            return;

        if (curr_node == list->head)
        {
            MemAllocsNode* prev_head = list->head;
            list->head = list->head->ptr_next;
            free(prev_head->data);
            free(prev_head);
        }

        else if (curr_node == list->tail)
        {
            MemAllocsNode* prev_tail = list->tail;

            MemAllocsNode* pre_tail = list->head;
            while (pre_tail->ptr_next != prev_tail)
                pre_tail = pre_tail->ptr_next;

            pre_tail->ptr_next = NULL;
            list->tail = pre_tail;
            free(prev_tail->data);
            free(prev_tail);
        }

        else
        {
            MemAllocsNode* pre_elem = list->head;
            while (pre_elem->ptr_next != curr_node)
                pre_elem = pre_elem->ptr_next;

            pre_elem->ptr_next = pre_elem->ptr_next->ptr_next;
            free(curr_node->data);
            free(curr_node);
        }
    }
}

// ============ Linked list implementation end=============

MemAllocsList* get_alloc_list()
{
    static MemAllocsList* list = NULL;
    if (!list) list = list_create();
    return list;
}

void* debug_malloc(size_t size, const char* file_name, const char* func_name, int src_line)
{
    void* block = malloc(size);

    MemAllocData* data = malloc(sizeof(MemAllocData));
    data->block = block;
    data->size = size;
    data->file_name = file_name;
    data->func_name = func_name;
    data->src_line = src_line;

    MemAllocsList* list = get_alloc_list();
    list_push_back(list, data);

    return block;
}

void* debug_calloc(size_t num, size_t size, const char* file_name, const char* func_name, int src_line)
{
    void* block = calloc(num, size);

    MemAllocData* data = malloc(sizeof(MemAllocData));
    data->block = block;
    data->size = num * size;
    data->file_name = file_name;
    data->func_name = func_name;
    data->src_line = src_line;

    MemAllocsList* list = get_alloc_list();
    list_push_back(list, data);

    return block;
}

void* debug_realloc(void* ptr, size_t new_size, const char* file_name, const char* func_name, int src_line)
{
    void* block = realloc(ptr, new_size);

    MemAllocData* data = malloc(sizeof(MemAllocData));
    MemAllocsList* list = get_alloc_list();

    // delete old allocation info
    data->block = ptr;
    list_remove(list, data);

    // create new allocation info
    data->block = block;
    data->size = new_size;
    data->file_name = file_name;
    data->func_name = func_name;
    data->src_line = src_line;
    list_push_back(list, data);

    return block;
}

void debug_free(void* ptr, const char* file_name, const char* func_name, int src_line)
{
    MemAllocsList* list = get_alloc_list();
    MemAllocData data;
    data.block = ptr;

    list_remove(list, &data);

    free(ptr);
}

void print_allocations_file(const char* file_path)
{
    FILE* f = fopen(file_path, "a");
    if (!f)
    {
        printf("unable to open file \"%s\" for writing!\n", file_path);
        return;
    }

    MemAllocsList* list = get_alloc_list();
    MemAllocsNode* curr_node = list->head;

    fprintf(f, "Allocation(s) that are not freed at the moment of this function call:\n");
    fprintf(f, "%-42s %-6s %-12s %-20s\n", "file", "line", "memblock", "bytes");
    while (curr_node)
    {
        MemAllocData* d = curr_node->data;
        fprintf(f, "%-42s %-6u %-12p %-20u\n", d->file_name, d->src_line, d->block, d->size);
        curr_node = curr_node->ptr_next;
    }
    fprintf(f, "\n");
}

void print_allocations_console()
{
    MemAllocsList* list = get_alloc_list();
    MemAllocsNode* curr_node = list->head;

    printf("Allocation(s) that are not freed at the moment of this function call:\n");
    printf("%-42s %-6s %-12s %-20s\n", "file", "line", "memblock", "bytes");
    while (curr_node)
    {
        MemAllocData* d = curr_node->data;
        printf("%-42s %-6u %-12p %-20u\n", d->file_name, d->src_line, d->block, d->size);
        curr_node = curr_node->ptr_next;
    }
    printf("\n");
}
