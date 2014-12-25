#include "phoenix.h"

int ph_listen_handler(int fd, int event, void *arg)
{
    int sfd;
    ph_pool_t *pool;
    ph_connection_t *conn;

    for ( ;; ) {
        sfd = accept(fd);
        if (sfd == -1) {
            break;
        }

        pool = ph_create_pool(PH_DEFAULT_POOL_SIZE);
        if (!pool) {
            goto failed;
        }

        conn = ph_palloc(sizeof(ph_connection_t));
        if (!conn) {
            goto failed;
        }

        conn->fd = sfd;
        conn->pool = pool;
        conn->buf = ph_palloc(conn->pool, PH_DEFAULT_HEADER_SIZE);
        conn->last = conn->buf;
        conn->end = conn->buf + PH_DEFAULT_HEADER_SIZE;

        event_add();

        continue;

failed:
        if (pool)
            ph_destroy_pool(pool);
        close(sfd);
        break;
    }

    return 0;
}

int ph_request_handler(int fd, int event, void *arg)
{
    ph_connection_t *conn = arg;

    
}

