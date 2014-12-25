#include "phoenix.h"

struct http_parser_settings setting = {
    .on_message_begin = ,
    .on_url = ,
    .on_status = ,
    .on_header_field = ,
    .on_header_value = ,
    .on_headers_complete = ,
    .on_body = ,
    .on_message_complete = ,
};

void ph_listen_handler(int fd, short event, void *arg)
{
    int sfd;
    struct sockaddr_in addr;
    int size = sizeof(struct sockaddr_in);
    ph_pool_t *pool;
    ph_connection_t *conn;

    for ( ;; ) {
        sfd = accept(fd, (struct sockaddr*)&addr, &size);
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
        conn->parser.data = conn;
        conn->buf = ph_palloc(conn->pool, PH_DEFAULT_HEADER_SIZE);
        conn->last = conn->buf;
        conn->end = conn->buf + PH_DEFAULT_HEADER_SIZE;

        event_set(&conn->event, conn->fd,
            EV_READ|EV_PERSIST, ph_request_handler, conn);
        event_base_set(base, &conn->event);
        event_add(&conn->event, NULL);

        continue;

failed:
        if (pool)
            ph_destroy_pool(pool);
        close(sfd);
        break;
    }

    return 0;
}

void ph_request_handler(int fd, short event, void *arg)
{
    ph_connection_t *conn = arg;
    int bytes, nread;
    char *nb;
    int last;

    for ( ;; ) {
        bytes = conn->end - conn->last;

        if (bytes <= 0) {
            if (conn->end - conn->buf >= PH_MAX_HEADER_SIZE) {
                ph_close_connection(conn);
                break;
            }

            nb = ph_palloc(conn->pool, PH_MAX_HEADER_SIZE);
            if (!nb) {
                ph_close_connection(conn);
                break;
            }

            last = conn->last - conn->buf;

            memcpy(nb, conn->buf, last);
            ph_pfree(conn->buf);

            conn->buf = nb;
            conn->last = conn->buf + last;
            conn->end = conn->buf + PH_MAX_HEADER_SIZE;

            bytes = conn->end - conn->last;
        }

        nread = read(conn->fd, conn->last, bytes);
        if (nread == 0) { /* connection was closed */
            ph_close_connection(conn);
            break;
        } else if (nread < 0) {
            break;
        }

        conn->last += nread;

        bytes = http_parser_execute(&conn->parser, &setting,
            conn->buf, conn->last - conn->buf);
        if (bytes > 0) { /* parse finished */
            
        }
    }
}

