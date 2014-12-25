
/*
 * memory pool design from Nginx
 */

#include <stdlib.h>
#include <string.h>
#include "pool.h"

ph_pool_t *ph_create_pool(size_t size)
{
    ph_pool_t  *p;

    if (!(p = malloc(size))) {
       return NULL;
    }

    p->last = (char *) p + sizeof(ph_pool_t);
    p->end = (char *) p + size;
    p->next = NULL;
    p->large = NULL;

    return p;
}


void ph_destroy_pool(ph_pool_t *pool)
{
    ph_pool_t        *p, *n;
    ph_pool_large_t  *l;

    for (l = pool->large; l; l = l->next) {
        if (l->alloc) {
            free(l->alloc);
        }
    }

    for (p = pool, n = pool->next; /* void */; p = n, n = n->next) {
        free(p);

        if (n == NULL) {
            break;
        }
    }
}


void *ph_palloc(ph_pool_t *pool, size_t size)
{
    char             *m;
    ph_pool_t        *p, *n;
    ph_pool_large_t  *large, *last;

    if (size <= (size_t) PH_MAX_ALLOC_FROM_POOL
        && size <= (size_t) (pool->end - (char *) pool) - sizeof(ph_pool_t))
    {
        for (p = pool, n = pool->next; /* void */; p = n, n = n->next) {
            m = ph_align(p->last);

            if ((size_t) (p->end - m) >= size) {
                p->last = m + size ;

                return m;
            }

            if (n == NULL) {
                break;
            }
        }

        /* allocate a new pool block */

        if (!(n = ph_create_pool((size_t) (p->end - (char *) p)))) {
            return NULL;
        }

        p->next = n;
        m = n->last;
        n->last += size;

        return m;
    }

    /* allocate a large block */

    large = NULL;
    last = NULL;

    if (pool->large) {
        for (last = pool->large; /* void */ ; last = last->next) {
            if (last->alloc == NULL) {
                large = last;
                last = NULL;
                break;
            }

            if (last->next == NULL) {
                break;
            }
        }
    }

    if (large == NULL) {
        if (!(large = ph_palloc(pool, sizeof(ph_pool_large_t)))) {
            return NULL;
        }

        large->next = NULL;
    }

    if (!(p = malloc(size))) {
        return NULL;
    }

    if (pool->large == NULL) {
        pool->large = large;

    } else if (last) {
        last->next = large;
    }

    large->alloc = p;

    return p;
}


void ph_pfree(ph_pool_t *pool, void *p)
{
    ph_pool_large_t  *l;

    for (l = pool->large; l; l = l->next) {
        if (p == l->alloc) {
            free(l->alloc);
            l->alloc = NULL;
        }
    }
}


void *ph_pcalloc(ph_pool_t *pool, size_t size)
{
    void *p;

    p = ph_palloc(pool, size);
    if (p) {
        memset(p, 0, size);
    }

    return p;
}
