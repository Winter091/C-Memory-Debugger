// to use unsafe c functions
#define _CRT_SECURE_NO_WARNINGS

#include "stdio.h"
#include "string.h"

// define in order to not affent actual alloc functions inside
// debug_alloc wrappers
#define MEM_DEBUG_DISABLE
#include "mem_debugger.h"

// parameters of bytes written afrer any allocated memory block
#define BOUND_CHECK_BYTES_COUNT 4
#define BOUND_CHECK_BYTE_VALUE 218

typedef unsigned long long ull;

// ======== Linked list for blocks implementation =========

typedef struct
{
    void* block;
    size_t size;
    const char* file_name;
    int src_line;
}
BlockData;

typedef struct LListBlocksNode
{
    BlockData* data;
    struct LListBlocksNode* ptr_next;
}
LListBlocksNode;

typedef struct
{
    LListBlocksNode *head, *tail;
    size_t size;
}
LListBlocks;

LListBlocks* llistblocks_create()
{
    LListBlocks* llist = malloc(sizeof(LListBlocks));
    llist->head = NULL;
    llist->tail = NULL;
    llist->size = 0;
    return llist;
}

void llistblocks_push_front(LListBlocks* llist, BlockData* elem)
{
    LListBlocksNode* node = malloc(sizeof(LListBlocksNode));
    node->data = elem;
    node->ptr_next = NULL;

    if (!llist->size)
    {
        llist->head = node;
        llist->tail = node;
    }

    else
    {
        LListBlocksNode* prev_head = llist->head;
        llist->head = node;
        llist->head->ptr_next = prev_head;
    }

    llist->size++;
}

int llistblocks_remove(LListBlocks* llist, void* block)
{
    if (!llist->size) return 0;

    if (llist->size == 1)
    {
        if (llist->head->data->block != block)
            return 0;

        free(llist->head->data);
        free(llist->head);
        llist->head = NULL;
        llist->tail = NULL;
        llist->size = 0;
        return 1;
    }

    // size > 1

    // if data is in head
    if (llist->head->data->block == block)
    {
        LListBlocksNode* prev_head = llist->head;
        llist->head = llist->head->ptr_next;
        llist->size--;
        free(prev_head->data);
        free(prev_head);
        return 1;
    }

    // elem should be in curr_node->ptr_next
    LListBlocksNode* curr_node = llist->head;
    while (curr_node->ptr_next && curr_node->ptr_next->data->block != block)
        curr_node = curr_node->ptr_next;

    // no such element is list?
    if (!curr_node->ptr_next) return 0;

    // if data is in tail
    if (curr_node->ptr_next == llist->tail)
    {
        LListBlocksNode* prev_tail = llist->tail;
        llist->tail = curr_node;
        llist->tail->ptr_next = NULL;
        free(prev_tail->data);
        free(prev_tail);
    }

    // data is not in tail
    else
    {
        LListBlocksNode* node_to_del = curr_node->ptr_next;
        curr_node->ptr_next = curr_node->ptr_next->ptr_next;
        free(node_to_del->data);
        free(node_to_del);
    }

    llist->size--;
    return 1;
}

BlockData* llistblocks_get(LListBlocks* llist, void* block)
{
    if (llist->size == 0) return NULL;

    if (llist->head->data->block == block)
        return llist->head->data;

    if (llist->tail->data->block == block)
        return llist->tail->data;

    LListBlocksNode* curr_node = llist->head;
    while (curr_node && curr_node->data->block != block)
        curr_node = curr_node->ptr_next;

    return curr_node ? curr_node->data : NULL;
}

void llistblocks_delete(LListBlocks* llist)
{
    LListBlocksNode* curr_node = llist->head;

    while (curr_node)
    {
        LListBlocksNode* next_node = curr_node->ptr_next;
        free(curr_node);
        curr_node = next_node;
    }

    free(llist);
}

// ====== Linked list for blocks implementation end =======

// == Linked list for heavy allocs implementation =========

typedef struct
{
    size_t bytes;
    const char* file_name;
    int src_line;
}
AllocData;

typedef struct LListAllocsNode
{
    AllocData* data;
    struct LListAllocsNode* ptr_next;
}
LListAllocsNode;

typedef struct
{
    LListAllocsNode *head, *tail;
    size_t size;
    size_t bytes_alloc_at_all;
}
LListAllocs;

LListAllocs* llistallocs_create()
{
    LListAllocs* llist = malloc(sizeof(LListAllocs));
    llist->head = NULL;
    llist->tail = NULL;
    llist->size = 0;
    llist->bytes_alloc_at_all = 0;
    return llist;
}

void llistallocs_push_front(LListAllocs* llist, AllocData* elem)
{
    LListAllocsNode* node = malloc(sizeof(LListAllocsNode));
    node->data = elem;
    node->ptr_next = NULL;

    if (!llist->size)
    {
        llist->head = node;
        llist->tail = node;
    }

    else
    {
        LListAllocsNode* prev_head = llist->head;
        llist->head = node;
        llist->head->ptr_next = prev_head;
    }

    llist->size++;
}

AllocData* llistallocs_get(LListAllocs* llist, BlockData* elem)
{
    if (llist->size == 0) return NULL;

    if (llist->head->data->src_line == elem->src_line
        && !strcmp(llist->head->data->file_name, elem->file_name))
        return llist->head->data;

    if (llist->tail->data->src_line == elem->src_line
        && !strcmp(llist->tail->data->file_name, elem->file_name))
        return llist->tail->data;

    LListAllocsNode* curr_node = llist->head;
    while (curr_node && (curr_node->data->src_line != elem->src_line || strcmp(llist->tail->data->file_name, elem->file_name)))
        curr_node = curr_node->ptr_next;

    return curr_node ? curr_node->data : NULL;
}

void llistallocs_update(LListAllocs* llist, BlockData* elem)
{
    llist->bytes_alloc_at_all += elem->size;

    if (llist->head->data->src_line == elem->src_line
        && !strcmp(llist->head->data->file_name, elem->file_name))
        llist->head->data->bytes += elem->size;

    else if (llist->tail->data->src_line == elem->src_line
        && !strcmp(llist->tail->data->file_name, elem->file_name))
        llist->tail->data->bytes += elem->size;

    else
    {
        LListAllocsNode* curr_node = llist->head;

        while (curr_node->data->src_line != elem->src_line || strcmp(llist->tail->data->file_name, elem->file_name))
            curr_node = curr_node->ptr_next;

        curr_node->data->bytes += elem->size;
    }
}

int compar_func(const void* p1, const void* p2)
{
    AllocData* d1 = *(AllocData**)p1;
    AllocData* d2 = *(AllocData**)p2;

    if (d1->bytes > d2->bytes) return -1;
    else if (d1->bytes == d2->bytes) return 0;
    else return 1;
}

AllocData** llistallocs_get_sorted_array(LListAllocs* llist)
{
    AllocData** arr = malloc(llist->size * sizeof(AllocData*));

    LListAllocsNode* curr_node = llist->head;
    for (int i = 0; i < llist->size; i++)
    {
        arr[i] = curr_node->data;
        curr_node = curr_node->ptr_next;
    }

    qsort(arr, llist->size, sizeof(AllocData*), compar_func);

    return arr;
}

void llistallocs_delete(LListAllocs* llist)
{
    LListAllocsNode* curr_node = llist->head;

    while (curr_node)
    {
        LListAllocsNode* next_node = curr_node->ptr_next;
        free(curr_node);
        curr_node = next_node;
    }

    free(llist);
}

// == Linked list for heavy allocs implementation end =====

// ================== Helper functions ====================

// singleton lists are controlled by these 2 functions
LListBlocks* get_llistblocks()
{
    static LListBlocks* list = NULL;
    if (!list) list = llistblocks_create();
    return list;
}

LListAllocs* get_llistallocs()
{
    static LListAllocs* list = NULL;
    if (!list) list = llistallocs_create();
    return list;
}

void warn_null_return(const char* from_func, const char* file_name, int src_line, size_t block_size)
{
    fprintf(stderr, "Warning!\n");
    fprintf(stderr, "%s at %s:%d returned NULL! (size was %llu)\n\n", 
            from_func, file_name, src_line, (ull)block_size);
    abort();
}

void warn_wrong_ptr(const char* from_func, const char* file_name, int src_line)
{
    fprintf(stderr, "Warning!\n");
    fprintf(stderr, "%s at %s:%d is used with pointer that does not point to active heap memory!\n\n",
            from_func, file_name, src_line);
    abort();
}

void warn_bound_violation(const char* file_name, int src_line, size_t block_size)
{
    fprintf(stderr, "Warning!\n");
    fprintf(stderr, "Out-of-bounds writing occured after memory block, allocated at:\n");
    fprintf(stderr, "%s:%d (%llu bytes)\n\n", file_name, src_line, (ull)block_size);
    abort();
}

BlockData* blockdata_create(void* block, size_t size, const char* file_name, int src_line)
{
    BlockData* block_data = malloc(sizeof(BlockData));
    block_data->block = block;
    block_data->size = size;
    block_data->file_name = file_name;
    block_data->src_line = src_line;
    return block_data;
}

void update_alloc_data(BlockData* block_data)
{
    AllocData* alloc_data = llistallocs_get(get_llistallocs(), block_data);

    if (!alloc_data)
    {
        alloc_data = malloc(sizeof(AllocData));
        alloc_data->bytes = 0;
        alloc_data->src_line = block_data->src_line;
        alloc_data->file_name = block_data->file_name;
        llistallocs_push_front(get_llistallocs(), alloc_data);
    }

    llistallocs_update(get_llistallocs(), block_data);
}

// ================ Helper functions end ==================

// ======= Debug versions of allocation functions =========

void* debug_malloc(size_t size, const char* file_name, int src_line)
{
    // additional space for bound-checking bytes
    void* block = malloc(size + BOUND_CHECK_BYTES_COUNT);

    if (!block)
        warn_null_return("malloc()", file_name, src_line, size);

    // set bytes for bound checking
    memset((unsigned char*)block + size, BOUND_CHECK_BYTE_VALUE, BOUND_CHECK_BYTES_COUNT);

    BlockData* block_data = blockdata_create(block, size, file_name, src_line);
    llistblocks_push_front(get_llistblocks(), block_data);
    update_alloc_data(block_data);

    return block;
}

void* debug_calloc(size_t num, size_t size, const char* file_name, int src_line)
{
    // additional space for bound-checking bytes
    void* block = calloc((num * size) + BOUND_CHECK_BYTES_COUNT, 1);

    if (!block)
        warn_null_return("calloc()", file_name, src_line, num * size);

    // set bytes for bound checking
    memset((unsigned char*)block + (num * size), BOUND_CHECK_BYTE_VALUE, BOUND_CHECK_BYTES_COUNT);

    BlockData* block_data = blockdata_create(block, num * size, file_name, src_line);
    llistblocks_push_front(get_llistblocks(), block_data);
    update_alloc_data(block_data);

    return block;
}

void* debug_realloc(void* ptr, size_t new_size, const char* file_name, int src_line)
{
    // this ptr should be in active allocs list
    BlockData* found_block = llistblocks_get(get_llistblocks(), ptr);

    // if it's not correct pointer to heap memory
    // (but nullptr is allowed)
    if (!found_block && ptr)
        warn_wrong_ptr("realloc()", file_name, src_line);

    void* block = realloc(ptr, new_size + BOUND_CHECK_BYTES_COUNT);

    if (!block)
        warn_null_return("realloc()", file_name, src_line, new_size);

    // set bytes for bound checking
    memset((unsigned char*)block + new_size, BOUND_CHECK_BYTE_VALUE, BOUND_CHECK_BYTES_COUNT);

    BlockData* block_data = blockdata_create(block, new_size, file_name, src_line);

    // updating existing info instead of removing
    // and inserting would be better approach, but whatever
    llistblocks_remove(get_llistblocks(), ptr);
    llistblocks_push_front(get_llistblocks(), block_data);

    update_alloc_data(block_data);

    return block;
}

void debug_free(void* ptr, const char* file_name, int src_line)
{
    // null ptrs do nothing
    if (ptr == NULL) return;

    // this ptr should be in active allocs list
    BlockData* found_block = llistblocks_get(get_llistblocks(), ptr);

    // if it's not correct pointer to heap memory
    if (!found_block)
        warn_wrong_ptr("free()", file_name, src_line);

    // check bytes after block, they shouldn't be changed
    unsigned char req_bytes[BOUND_CHECK_BYTES_COUNT];
    memset(req_bytes, BOUND_CHECK_BYTE_VALUE, BOUND_CHECK_BYTES_COUNT);

    if (memcmp((unsigned char*)ptr + found_block->size,
               req_bytes, BOUND_CHECK_BYTES_COUNT))
    {
        warn_bound_violation(found_block->file_name,
                             found_block->src_line, found_block->size);
    }

    // if all good, remove ptr form list of active allocs
    llistblocks_remove(get_llistblocks(), ptr);

    free(ptr);
}

// ===== Debug versions of allocation functions end =======

void print_to_stream(FILE* stream)
{
    // print info about memory leaks
    fprintf(stream, "%s", "Current unfreed allocations:\n");

    fprintf(stream, "%-42.42s %-5.5s %-10.10s %-10.10s\n",
            "Source file", "Line", "Mem. ptr", "Bytes");

    size_t total_size = 0;
    size_t blocks_count = 0;

    LListBlocks* blocks = get_llistblocks();
    LListBlocksNode* curr_node = blocks->head;

    while (curr_node)
    {
        fprintf(stream, "%-42.42s %-5d %-10p %-10llu\n",
                curr_node->data->file_name,
                curr_node->data->src_line,
                curr_node->data->block,
                (ull)curr_node->data->size);

        total_size += curr_node->data->size;
        blocks_count++;

        curr_node = curr_node->ptr_next;
    }

    fprintf(stream, "\nUnfreed total: %llu bytes from %llu allocation(s)\n\n",
            (ull)total_size, (ull)blocks_count);

    // print info about heavy-hitter allocations
    fprintf(stream, "Top-5 heaviest allocations so far:\n");

    fprintf(stream, "%-42.42s %-5.5s %-10.10s %-10.10s\n",
            "Source file", "Line", "Bytes", "Percentage");

    AllocData** arr = llistallocs_get_sorted_array(get_llistallocs());
    size_t arr_size = get_llistallocs()->size;
    size_t total_bytes = get_llistallocs()->bytes_alloc_at_all;

    for (int i = 0; i < 5 && i < arr_size; i++)
    {
        fprintf(stream, "%-42.42s %-5d %-10llu %.2lf%%\n",
                arr[i]->file_name,
                arr[i]->src_line,
                (ull)arr[i]->bytes,
                (double)arr[i]->bytes / total_bytes * 100.0);
    }

    free(arr);

    fprintf(stream, "\nAllocated total: %llu bytes\n\n", (ull)total_bytes);
}

// only this function is available to user
void mem_debugger_dump_info(int use_file, const char* file_name)
{
    if (use_file && !file_name)
        return;

    FILE* stream = use_file ? fopen(file_name, "a") : stderr;

    if (stream)
        print_to_stream(stream);

    if (use_file)
        fclose(stream);
}
