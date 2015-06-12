#ifndef PHX_POOL_H
#define PHX_POOL_H

typedef struct phx_pool_s  phx_pool_t;
typedef struct phx_pool_large_s  phx_pool_large_t;

struct phx_pool_large_s {
    phx_pool_large_t   *next;
    void              *alloc;
};

struct phx_pool_s {
    char              *last;
    char              *end;
    phx_pool_t         *next;
    phx_pool_large_t   *large;
};

#define PHX_MAX_ALLOC_FROM_POOL  4095
#define PHX_DEFAULT_POOL_SIZE   (16 * 1024)

#define PHX_ALIGN       (sizeof(unsigned long) - 1)
#define PHX_ALIGN_CAST  (unsigned long)

#define phx_align(p)    (char *)((PHX_ALIGN_CAST p + PHX_ALIGN) & (~PHX_ALIGN))


phx_pool_t *phx_create_pool(size_t size);
void phx_destroy_pool(phx_pool_t *pool);
void *phx_palloc(phx_pool_t *pool, size_t size);
void phx_pfree(phx_pool_t *pool, void *p);
void *phx_pcalloc(phx_pool_t *pool, size_t size);

#endif
