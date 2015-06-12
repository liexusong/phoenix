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


void phx_listen(int fd, short event, void *arg)
{
    int sfd;
    struct sockaddr_in addr;
    int size = sizeof(struct sockaddr_in);
    phx_pool_t *pool;
    phx_connection_t *conn;
    char *buf;

    for ( ;; ) {

        sfd = accept(fd, (struct sockaddr*)&addr, &size);
        if (sfd == -1) {
            phx_log_error("Failed to accpet client from listen socket");
            break;
        }

        if (!(pool = phx_create_pool(PHX_DEFAULT_POOL_SIZE)) ||
            !(conn = phx_palloc(sizeof(*conn))) ||
            !(buf = phx_palloc(PHX_DEFAULT_POOL_SIZE)))
        {
            goto failed;
        }

        conn->sock = sfd;
        conn->pool = pool;
        conn->parser.data = conn;
        conn->buf = buf;
        conn->last = conn->buf;
        conn->end = conn->buf + PHX_DEFAULT_POOL_SIZE;

        event_set(&conn->event, conn->sock,
            EV_READ|EV_PERSIST, phx_process_request, conn);
        event_base_set(base, &conn->event);
        event_add(&conn->event, NULL);

        continue;

failed:
        if (pool)
            phx_destroy_pool(pool);
        close(sfd);
        break;
    }

    return 0;
}


void phx_process_request(int fd, short event, void *arg)
{
    phx_connection_t *conn = arg;
    int bytes, nread;
    char *nbuf;
    int last;

    if (conn->sock != fd) {
        phx_log_error("Libevent fd -> (%d) not equls conn::sock -> (%d)", fd, conn->sock);
        return;
    }

    for ( ;; ) {

        bytes = conn->end - conn->last;

        if (bytes <= 0) { /* resize request buffer */

            /* client header buffer size exceeding the max size */
            if (conn->end - conn->buf >= PHX_MAX_HEADER_SIZE) {
                phx_close_connection(conn);
                break;
            }

            /* alloc the max buffer size */
            nbuf = phx_palloc(conn->pool, PHX_MAX_HEADER_SIZE);
            if (!nbuf) {
                phx_close_connection(conn);
                break;
            }

            last = conn->last - conn->buf;

            memcpy(nbuf, conn->buf, last);
            phx_pfree(conn->buf);

            conn->buf = nbuf;
            conn->last = conn->buf + last;
            conn->end = conn->buf + PHX_MAX_HEADER_SIZE;

            bytes = conn->end - conn->last;
        }

        nread = read(conn->sock, conn->last, bytes);
        if (nread == 0) { /* connection was closed */
            phx_close_connection(conn);
            break;
        } else if (nread < 0) {
            break;
        }

        conn->last += nread;

        bytes = http_parser_execute(&conn->parser, &setting,
            conn->buf, conn->last - conn->buf);
        if (bytes > 0) { /* parse OK */
            
        }
    }
}


void phx_send_response(int fd, short event, void *arg)
{
    
}


void phx_close_connection(phx_connection_t *conn)
{
    close(conn->sock);
    phx_free_pool(conn->pool);
}
