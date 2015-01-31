#ifndef DDEBUG
#define DDEBUG 0
#endif


#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>


#if DDEBUG
#   define dd(...) \
        fprintf(stderr, "wrapalloc: "); \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, " at %s line %d.\n", __FILE__, __LINE__)
#else
#   define dd(...)
#endif


#define init_libc_handle() \
        if (libc_handle == NULL) { \
            libc_handle = RTLD_NEXT; \
        }

#define init_orig(name) \
        init_libc_handle(); \
        if (orig_ ## name == NULL) { \
            orig_ ## name = dlsym(libc_handle, #name); \
            if (orig_ ## name == NULL) { \
                fprintf(stderr, "mockeagain: could not find the underlying " #name \
                        ": %s\n", dlerror()); \
                exit(1); \
            } \
        }


static void *libc_handle = NULL;


typedef void *(*malloc_func_t)(size_t);
typedef void *(*realloc_func_t)(void *, size_t);
typedef void *(*calloc_func_t)(size_t, size_t);
typedef int (*posix_memalign_func_t)(void **, size_t, size_t);
typedef void *(*memalign_func_t)(size_t, size_t);
typedef void *(*aligned_alloc_func_t)(size_t, size_t);
typedef void *(*valloc_func_t)(size_t);
typedef void *(*pvalloc_func_t)(size_t);
typedef void (*free_func_t)(void *);


void __attribute__ ((noinline)) probe_alloc_event(void *memptr, size_t bytes)
{
}

void __attribute__ ((noinline)) probe_free_event(void *memptr)
{
}

void *
malloc(size_t bytes)
{
    void                    *ptr;
    static malloc_func_t     orig_malloc = NULL;

    init_orig(malloc);
    ptr = orig_malloc(bytes);
    probe_alloc_event(ptr, bytes);
    return ptr;

}


void *
realloc(void *oldmem, size_t bytes)
{
    void                     *ptr;
    static realloc_func_t     orig_realloc = NULL;

    init_orig(realloc);
    probe_free_event(oldmem);
    ptr = orig_realloc(oldmem, bytes);
    probe_alloc_event(ptr, bytes);
    return ptr;
}


void *
calloc(size_t n, size_t elem_size)
{
    void                    *ptr;
    static calloc_func_t     orig_calloc = NULL;

    init_orig(calloc);
    ptr = orig_calloc(n, elem_size);
    probe_alloc_event(ptr, n * elem_size);
    return ptr;
}


int
posix_memalign (void **memptr, size_t alignment, size_t size)
{
    int                              rc;
    static posix_memalign_func_t     orig_posix_memalign = NULL;

    init_orig(posix_memalign);
    rc = orig_posix_memalign(memptr, alignment, size);
    probe_alloc_event(*memptr, size);
    return rc;
}


void *
memalign(size_t alignment, size_t bytes)
{
    void                      *ptr;
    static memalign_func_t     orig_memalign = NULL;

    init_orig(memalign);
    ptr = orig_memalign(alignment, bytes);
    probe_alloc_event(ptr, bytes);
    return ptr;
}


void *
aligned_alloc(size_t alignment, size_t bytes)
{
    void                      *ptr;
    static memalign_func_t     orig_aligned_alloc = NULL;

    init_orig(aligned_alloc);
    ptr = orig_aligned_alloc(alignment, bytes);
    probe_alloc_event(ptr, bytes);
    return ptr;
}


void *
valloc(size_t bytes)
{
    void                    *ptr;
    static valloc_func_t     orig_valloc = NULL;

    init_orig(valloc);
    ptr = orig_valloc(bytes);
    probe_alloc_event(ptr, bytes);
    return ptr;
}


void *
pvalloc(size_t bytes)
{
    void                     *ptr;
    static pvalloc_func_t     orig_pvalloc = NULL;

    init_orig(pvalloc);
    ptr = orig_pvalloc(bytes);
    probe_alloc_event(ptr, bytes);
    return ptr;
}


void
free(void *ptr)
{
    static free_func_t     orig_free = NULL;

    init_orig(free);
    orig_free(ptr);
    probe_free_event(ptr);
}
