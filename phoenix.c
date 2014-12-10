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

typedef struct {
	int fd;
	struct sockaddr_in addr;
} ph_connection_t;


static char http200[] = "HTTP/1.0 200 OK\r\nContent-length: 15\r\n\r\nHello Liexusong";

void ph_service_handler(void *arg)
{
	ph_connection_t *conn = arg;
	char buf[1024];
	int bytes;

	bytes = lthread_recv(conn->fd, buf, 1024, 0, PH_DEFAULT_TIMEOUT);
	if (bytes == -2) { /* timeout */
		close(conn->fd);
		free(conn);
		return;
	}

	lthread_send(conn->fd, http200, sizeof(http200) - 1, 0);

	close(conn->fd);
	free(conn);
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

        (void)lthread_create(&clt, 
            (void *)ph_service_handler, (void *)conn);
	}
}


int main(int argc, char *argv[])
{
	lthread_t *lt = NULL;

	lthread_create(&lt, (void *)ph_listen_handler, NULL);
	lthread_run();

	return 0;
}
