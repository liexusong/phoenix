/*
 * Phoenix - a fast and light web server
 * Copyright (C) 2014, Liexusong <liexusong@qq.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <lthread.h>


#define  PH_DEFAULT_PORT     8080
#define  PH_DEFAULT_BACKLOG  1024
#define  PH_DEFAULT_TIMEOUT  3000
#define  PH_DEFAULT_BUFSIZE  256
#define  PH_DEFAULT_INCSIZE  128
#define  PH_MAX_HEADER_SIZE  2048


typedef enum {
    ph_header_s0,
    ph_header_s1,
    ph_header_s2,
    ph_header_s3
} ph_header_state;

typedef struct {
    int fd;
    struct sockaddr_in addr;
    char *buf;
    char *pos;
    char *last;
    char *end;
    ph_header_state state;
} ph_connection_t;


static char http200[] = "HTTP/1.0 200 OK\r\nContent-length: 15\r\n\r\nHello Liexusong";

int ph_read_header(ph_connection_t *conn)
{
    while (conn->pos < conn->end) {

        switch (conn->state) {
        case ph_header_s0:
            if (*conn->pos == '\r') {
                conn->state = ph_header_s1;
            } else {
                conn->state = ph_header_s0;
            }
            break;

        case ph_header_s1:
            if (*conn->pos == '\n') {
                conn->state = ph_header_s2;
            } else {
                conn->state = ph_header_s0;
            }
            break;

        case ph_header_s2:
            if (*conn->pos == '\r') {
                conn->state = ph_header_s3;
            } else {
                conn->state = ph_header_s0;
            }
            break;

        case ph_header_s3:
            if (*conn->pos == '\n') {
                conn->pos++; /* move the end of header */
                return 0;
            } else {
                conn->state = ph_header_s0;
            }
            break;
        }

        conn->pos++;
    }

    return -1;
}

void ph_service_handler(void *arg)
{
    ph_connection_t *conn = arg;
    int bytes;

    conn->buf = malloc(PH_DEFAULT_BUFSIZE);
    if (!conn->buf) {
        close(conn->fd);
        free(conn);
        return;
    }

    conn->pos = conn->buf;
    conn->last = conn->buf;
    conn->end = conn->buf + PH_DEFAULT_BUFSIZE;

    for ( ;; ) {

        if (conn->end - conn->last <= 0) { /* resize read buffer */
            if (conn->end - conn->buf >= PH_MAX_HEADER_SIZE) {
                goto out;
            }

            int   size = (conn->end - conn->buf) + PH_DEFAULT_INCSIZE;
            char *temp = realloc(conn->buf, size);

            if (temp) {
                int pos = conn->pos - conn->buf;
                int last = conn->last - conn->buf;

                conn->buf = temp;
                conn->pos = conn->buf + pos;
                conn->last = conn->buf + last;
                conn->end = conn->buf + size;

            } else {
                goto out;
            }
        }

        bytes = lthread_recv(conn->fd, conn->last,
            conn->end - conn->last, 0, PH_DEFAULT_TIMEOUT);
        if (bytes == -2) { /* timeout */
            goto out;
        }

        conn->last += bytes;

        if (!ph_read_header(conn)) { /* read header finish */
            break;
        }
    }

    fprintf(stderr, "Header length: %d, %s\n", conn->pos - conn->buf, conn->buf);

    lthread_send(conn->fd, http200, sizeof(http200) - 1, 0);

out:
    close(conn->fd);
    free(conn->buf);
    free(conn);
    return;
}


/*
 * accept client and create new lthread
 */
void ph_listen_handler(void *arg)
{
    int sfd, cfd;
    int opt = 1;
    struct sockaddr_in sin = {0};
    struct sockaddr_in addr = {0};
    socklen_t len = sizeof(addr);
    int retval;
    lthread_t *clt;
    ph_connection_t *conn;

    DEFINE_LTHREAD; /* set lthread function name */

    sfd = lthread_socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sfd == -1) {
        fprintf(stderr, "Unable to create socket\n");
        return;
    }

    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt,
        sizeof(int)) == -1)
    {
        close(sfd);
        fprintf(stderr, "Unable to set SOREUSEADDR on socket\n");
        return;
    }

    memset(&sin, 0, sizeof(sin));

    sin.sin_family = PF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(PH_DEFAULT_PORT);

    retval = bind(sfd, (struct sockaddr *)&sin, sizeof(sin));
    if (retval == -1) {
        close(sfd);
        fprintf(stderr, "Unable to bind on port %d\n", PH_DEFAULT_PORT);
        return;
    }

    listen(sfd, PH_DEFAULT_BACKLOG);

    for ( ;; ) {

        cfd = lthread_accept(sfd, (struct sockaddr *)&addr, &len);
        if (cfd == -1) {
            fprintf(stderr, "Unable to accept connection\n");
            continue;
        }

        conn = malloc(sizeof(*conn));
        if (!conn) {
            close(cfd);
            continue;
        }

        conn->addr = addr;
        conn->fd = cfd;
        conn->state = ph_header_s0;

        (void)lthread_create(&clt, (void *)ph_service_handler, (void *)conn);
    }

    return;
}


int main(int argc, char *argv[])
{
    lthread_t *lt = NULL;

    lthread_create(&lt, (void *)ph_listen_handler, NULL);
    lthread_run();

    return 0;
}
