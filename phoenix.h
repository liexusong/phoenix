#ifndef __PHOENIX_H
#define __PHOENIX_H

#include <event.h>
#include "pool.h"
#include "http_parser.h"

#define PH_DEFAULT_HEADER_SIZE  128
#define PH_MAX_HEADER_SIZE      2048


typedef struct ph_connection_s {
    int fd;
    ph_pool_t *pool;
    struct event event;
    struct http_parser parser;
    char *buf;
    char *last;
    char *end;
} ph_connection_t;

#endif

