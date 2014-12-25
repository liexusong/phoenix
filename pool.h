#ifndef PH_POOL_H
#define PH_POOL_H

typedef struct ph_pool_s  ph_pool_t;
typedef struct ph_pool_large_s  ph_pool_large_t;

struct ph_pool_large_s {
    ph_pool_large_t   *next;
    void              *alloc;
};

struct ph_pool_s {
    char              *last;
    char              *end;
    ph_pool_t         *next;
    ph_pool_large_t   *large;
};

#define PH_MAX_ALLOC_FROM_POOL  4095
#define PH_DEFAULT_POOL_SIZE   (16 * 1024)

#define PH_ALIGN       (sizeof(unsigned long) - 1)
#define PH_ALIGN_CAST  (unsigned long)

#define ph_align(p)    (char *)((PH_ALIGN_CAST p + PH_ALIGN) & ~PH_ALIGN)


ph_pool_t *ph_create_pool(size_t size);
void ph_destroy_pool(ph_pool_t *pool);
void *ph_palloc(ph_pool_t *pool, size_t size);
void ph_pfree(ph_pool_t *pool, void *p);
void *ph_pcalloc(ph_pool_t *pool, size_t size);

#endif
