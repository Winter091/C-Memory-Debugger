#include "stdio.h"
#include "string.h"

#define MEM_DEBUG_DISABLE
#include "mem_debugger.h"

#define BOUND_CHECK_BYTES_COUNT 4
#define BOUND_CHECK_BYTE_VALUE 218

#define USER_MEM_PTR(x) ((unsigned char*)x + 4)
#define DEBUG_MEM_PTR(x) ((unsigned char*)x - 4)

// ========= Linked list for ptrs implementation ==========

typedef struct LListPtrsNode
{
    void* data;
    struct LListPtrsNode* ptr_next;
}
LListPtrsNode;

typedef struct
{
    LListPtrsNode *head, *tail;
    size_t size;
}
LListPtrs;

LListPtrs* llistptrs_create()
{
    LListPtrs* llist = malloc(sizeof(LListPtrs));
    llist->head = NULL;
    llist->tail = NULL;
    llist->size = 0;
    return llist;
}

void llistptrs_push_back(LListPtrs* llist, void* elem)
{
    LListPtrsNode* node = malloc(sizeof(LListPtrsNode));
    node->data = elem;
    node->ptr_next = NULL;

    // size = 0
    if (!llist->head)
    {
        llist->head = node;
        llist->tail = node;
    }

    // size > 0
    else
    {
        LListPtrsNode* prev_tail = llist->tail;
        llist->tail = node;
        prev_tail->ptr_next = node;
    }

    llist->size++;
}

void llistptrs_push_front(LListPtrs* llist, void* elem)
{
    LListPtrsNode* node = malloc(sizeof(LListPtrsNode));
    node->data = elem;
    node->ptr_next = NULL;

    if (!llist->size)
    {
        llist->head = node;
        llist->tail = node;
    }

    else
    {
        LListPtrsNode* prev_head = llist->head;
        llist->head = node;
        llist->head->ptr_next = prev_head;
    }

    llist->size++;
}

void* llistptrs_pop_front(LListPtrs* llist)
{
    if (llist->size == 0) return 0;

    if (llist->size == 1)
    {
        void* elem = llist->head->data;
        free(llist->head);
        llist->head = NULL;
        llist->tail = NULL;
        llist->size = 0;
        return elem;
    }

    // size > 1
    void* elem = llist->head->data;
    LListPtrsNode* prev_head = llist->head;
    llist->head = llist->head->ptr_next;
    llist->size--;
    free(prev_head);
    return elem;
}

int llistptrs_remove(LListPtrs* llist, void* elem)
{
    if (!llist->size) return 0;

    if (llist->size == 1)
    {
        if (llist->head->data != elem)
            return 0;

        free(llist->head);
        llist->head = NULL;
        llist->tail = NULL;
        llist->size = 0;
        return 1;
    }

    // size > 1
    else
    {
        // if data is in head
        if (llist->head->data == elem)
        {
            LListPtrsNode* prev_head = llist->head;
            llist->head = llist->head->ptr_next;
            llist->size--;
            free(prev_head);
            return 1;
        }

        // elem should be in curr_node->ptr_next
        LListPtrsNode* curr_node = llist->head;
        while (curr_node->ptr_next && curr_node->ptr_next->data != elem)
            curr_node = curr_node->ptr_next;

        // no such element is list?
        if (!curr_node->ptr_next) return 0;

        // if data is in tail
        if (curr_node->ptr_next == llist->tail)
        {
            LListPtrsNode* prev_tail = llist->tail;
            llist->tail = curr_node;
            llist->tail->ptr_next = NULL;
            free(prev_tail);
        }

        // data is not in tail
        else
        {
            LListPtrsNode* node_to_del = curr_node->ptr_next;
            curr_node->ptr_next = curr_node->ptr_next->ptr_next;
            free(node_to_del);
        }

        llist->size--;
        return 1;
    }
}

int llistptrs_contains(LListPtrs* llist, void* elem)
{
    if (llist->size == 0) return 0;

    if (llist->head->data == elem || llist->tail->data == elem)
        return 1;

    LListPtrsNode* curr_node = llist->head;
    while (curr_node && curr_node->data != elem)
        curr_node = curr_node->ptr_next;

    return curr_node ? 1 : 0;
}

void llistptrs_print(LListPtrs* llist)
{
    LListPtrsNode* curr_node = llist->head;

    while (curr_node)
    {
        printf("%p ", curr_node->data);
        curr_node = curr_node->ptr_next;
    }
    printf("\n");
}

void llistptrs_delete(LListPtrs* llist)
{
    LListPtrsNode* curr_node = llist->head;

    while (curr_node)
    {
        LListPtrsNode* next_node = curr_node->ptr_next;
        free(curr_node);
        curr_node = next_node;
    }

    free(llist);
}

// ======== Linked list for ptrs implementation end =======

// == Linked list for heavy allocs implementation =========

typedef struct
{
    size_t bytes_allocated;
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
    return llist;
}

void llistallocs_push_back(LListAllocs* llist, AllocData* elem)
{
    LListAllocsNode* node = malloc(sizeof(LListAllocsNode));
    node->data = elem;
    node->ptr_next = NULL;

    // size = 0
    if (!llist->head)
    {
        llist->head = node;
        llist->tail = node;
    }

    // size > 0
    else
    {
        LListAllocsNode* prev_tail = llist->tail;
        llist->tail = node;
        prev_tail->ptr_next = node;
    }

    llist->size++;
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

AllocData* llistallocs_pop_front(LListAllocs* llist)
{
    if (llist->size == 0) return 0;

    if (llist->size == 1)
    {
        AllocData* elem = llist->head->data;
        free(llist->head);
        llist->head = NULL;
        llist->tail = NULL;
        llist->size = 0;
        return elem;
    }

    // size > 1
    AllocData* elem = llist->head->data;
    LListAllocsNode* prev_head = llist->head;
    llist->head = llist->head->ptr_next;
    llist->size--;
    free(prev_head);
    return elem;
}

int llistallocs_remove(LListAllocs* llist, AllocData* elem)
{
    if (!llist->size) return 0;

    if (llist->size == 1)
    {
        if (llist->head->data != elem)
            return 0;

        free(llist->head);
        llist->head = NULL;
        llist->tail = NULL;
        llist->size = 0;
        return 1;
    }

    // size > 1
    else
    {
        // if data is in head
        if (llist->head->data == elem)
        {
            LListAllocsNode* prev_head = llist->head;
            llist->head = llist->head->ptr_next;
            llist->size--;
            free(prev_head);
            return 1;
        }

        // elem should be in curr_node->ptr_next
        LListAllocsNode* curr_node = llist->head;
        while (curr_node->ptr_next && curr_node->ptr_next->data != elem)
            curr_node = curr_node->ptr_next;

        // no such element is list?
        if (!curr_node->ptr_next) return 0;

        // if data is in tail
        if (curr_node->ptr_next == llist->tail)
        {
            LListAllocsNode* prev_tail = llist->tail;
            llist->tail = curr_node;
            llist->tail->ptr_next = NULL;
            free(prev_tail);
        }

        // data is not in tail
        else
        {
            LListAllocsNode* node_to_del = curr_node->ptr_next;
            curr_node->ptr_next = curr_node->ptr_next->ptr_next;
            free(node_to_del);
        }

        llist->size--;
        return 1;
    }
}

int llistallocs_contains(LListAllocs* llist, AllocData* elem)
{
    if (llist->size == 0) return 0;

    if (llist->head->data == elem || llist->tail->data == elem)
        return 1;

    LListAllocsNode* curr_node = llist->head;
    while (curr_node && curr_node->data != elem)
        curr_node = curr_node->ptr_next;

    return curr_node ? 1 : 0;
}

void llistallocs_print(LListAllocs* llist)
{
    LListAllocsNode* curr_node = llist->head;

    while (curr_node)
    {
        printf("%d ", curr_node->data);
        curr_node = curr_node->ptr_next;
    }
    printf("\n");
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

LListPtrs* get_llistptrs()
{
    static LListPtrs* list = NULL;
    if (!list) list = llistptrs_create();
    return list;
}

LListAllocs* get_llistallocs()
{
    static LListAllocs* list = NULL;
    if (!list) list = llistallocs_create();
    return list;
}

void* debug_malloc(size_t size, const char* file_name, int src_line)
{
    // additional space for block size and bound-checking bytes
    void* block = malloc(4 + size + BOUND_CHECK_BYTES_COUNT);

    if (!block)
    {
        printf("Warning! malloc returned nullptr\n");
        abort();
    }

    llistptrs_push_back(get_llistptrs(), USER_MEM_PTR(block));

    // copy block size to the first 4 bytes of block
    uint32_t size4b = block ? size : 0;
    memcpy(block, &size4b, 4);

    // set bytes for bound checking
    memset(block + 4 + size, BOUND_CHECK_BYTE_VALUE, BOUND_CHECK_BYTES_COUNT);

    return USER_MEM_PTR(block);
}

void* debug_calloc(size_t num, size_t size, const char* file_name, int src_line)
{
    // additional space for block size and bound-checking bytes
    void* block = calloc(4 + (num * size) + BOUND_CHECK_BYTES_COUNT, 1);

    if (!block)
    {
        printf("Warning! calloc returned nullptr\n");
        abort();
    }

    llistptrs_push_back(get_llistptrs(), USER_MEM_PTR(block));

    // copy block size to the first 4 bytes of block
    uint32_t size4b = block ? (num * size) : 0;
    memcpy(block, &size4b, 4);

    // set bytes for bound checking
    memset(block + 4 + (num * size), BOUND_CHECK_BYTE_VALUE, BOUND_CHECK_BYTES_COUNT);

    return USER_MEM_PTR(block);
}

void* debug_realloc(void* ptr, size_t new_size, const char* file_name, int src_line)
{
    void* block = realloc(ptr, new_size);



    return block;
}

void debug_free(void* ptr, const char* file_name, int src_line)
{
    // if it's not correct pointer to heap memory
    if (!llistptrs_contains(get_llistptrs(), ptr))
    {
        printf("FREEING WRONT PTR!\n");
        abort();
    }

    // get 'user-block' size
    uint32_t size = *(uint32_t*)((char*)ptr - 4);

    // check bytes after user-block, they shouldn't be changed
    unsigned char req_bytes[BOUND_CHECK_BYTES_COUNT];
    memset(req_bytes, BOUND_CHECK_BYTE_VALUE, BOUND_CHECK_BYTES_COUNT);
    if (memcmp((unsigned char*)ptr + size, req_bytes, BOUND_CHECK_BYTES_COUNT))
    {
        printf("MEMORY BOUND VIOLATION!\n");
        abort();
    }

    llistptrs_remove(get_llistptrs(), ptr);

    free(DEBUG_MEM_PTR(ptr));
}

void print_allocations_file(const char* file_path)
{

}

void print_allocations_console()
{

}
