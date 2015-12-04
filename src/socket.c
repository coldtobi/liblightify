/*
  liblightify -- library to control OSRAM's LIGHTIFY

Copyright (c) 2015, Tobias Frost <tobi@coldtobi.de>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the author nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "liblightify-private.h"

#include "socket.h"
#include "context.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int write_to_socket(struct lightify_ctx *ctx, unsigned char *msg, size_t size) {

	int n;
	int fd = lightify_skt_getfd(ctx);
	if (fd < 0) return -EINVAL;
	size_t m = size; /*<< current position */
	struct timeval to;

#ifdef ENABLE_DEBUG
	unsigned char *msg_ = msg;
#endif

	do {
		n = write(fd, msg, m);
		if (n < 0) {
			if (EINTR == errno ) continue;
			if (errno != EWOULDBLOCK && errno != EAGAIN) return -errno;
		}
		m -= n;
		msg += n;
		if (m) {
			/* check if O_NONBLOCK is set; in this case we retry */
			n = fcntl(fd, F_GETFL, 0);
			if (-1 == n) return -errno;
			if (0 == (n & O_NONBLOCK)) {
				/* short write. return what we've got done */
				return size-m;
			}
			/* non-blocking I/O confirmed -- setup timeout and retry when socket is ready */
			to = lightify_skt_getiotimeout(ctx);
			fd_set myset;
			FD_ZERO(&myset);
			FD_SET(fd, &myset);
			n = select(fd, NULL, &myset, NULL, &to);
			if (n < 0 && errno != EINTR) return -errno;
			if (n == 0) return -ETIMEDOUT;
		}
	} while (m);

#ifdef ENABLE_DEBUG
	if (lightify_get_log_priority(ctx) >= LOG_DEBUG) {
		unsigned int j = 0, k;
		char buf[80];
		buf[0] = 0;

		for (k = 0; k < size; k++, j++) {
			if (8 == j) {
				dbg(ctx,"> %s\n",buf);
				buf[0] = 0;
				j = 0;
			}
			sprintf(buf + strlen(buf), " 0x%02x,", msg_[k]);
		}
		dbg(ctx,"> %s\n",buf);
	}
#endif
	return size;
}

int read_from_socket(struct lightify_ctx *ctx, unsigned char *msg, size_t size ) {

	int n;
	int i;
	int fd = lightify_skt_getfd(ctx);
	if (fd < 0) return -EINVAL;
	size_t m = size;
	struct timeval to;

#ifdef ENABLE_DEBUG
	unsigned char *msg_ = msg;
#endif

	do {
		n = read(fd, msg, m);
		if (n == -1) {
			if(errno == EINTR) continue;
			if(errno != EWOULDBLOCK && errno != EAGAIN) return -errno;
		}
		// EOF
		if (n == 0) return size-m;

		m -= n;
		msg += n;

		if (m) {
			/* check if O_NONBLOCK is set; in this case we retry */
			n = fcntl(fd, F_GETFL, 0);
			if (-1 == n) return -errno;
			if (0 == (n & O_NONBLOCK)) {
				/* short read. return what we've got done */
				return size-m;
			}
			to = lightify_skt_getiotimeout(ctx);
			fd_set myset;
			FD_ZERO(&myset);
			FD_SET(fd, &myset);
			i = select(fd, NULL, &myset, NULL, &to);
			if (i < 0 && errno != EINTR) {
				return -errno;
			}
			if (m == 0 && i == 0) {
				return -ETIMEDOUT;
			} else {
				return size-m;
			}
		}
	} while (m);

#ifdef ENABLE_DEBUG
	if (lightify_get_log_priority(ctx) >= LOG_DEBUG) {
		unsigned int j = 0, k;
		char buf[80];
		buf[0] = 0;

		for (k = 0; k < size; k++, j++) {
			if (8 == j) {
				dbg(ctx,"< %s\n",buf);
				buf[0] = 0;
				j = 0;
			}
			sprintf(buf + strlen(buf), " 0x%02x,", msg_[k]);
		}
		dbg(ctx,"< %s\n",buf);
	}
#endif

	return size-m;
}

LIGHTIFY_EXPORT int lightify_skt_setfd(struct lightify_ctx *ctx, int socket) {
	if (!ctx) return -EINVAL;
	ctx->socket = socket;
	return 0;
}

LIGHTIFY_EXPORT int lightify_skt_getfd(struct lightify_ctx *ctx) {
	if (!ctx) return -EINVAL;
	return ctx->socket;
}

LIGHTIFY_EXPORT int lightify_skt_setiotimeout(struct lightify_ctx *ctx, struct timeval tv) {
	if (!ctx) return -EINVAL;
	ctx->iotimeout = tv;
	return 0;
}

LIGHTIFY_EXPORT struct timeval lightify_skt_getiotimeout(struct lightify_ctx *ctx) {
	if (!ctx) {
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		return tv;
	}
	return ctx->iotimeout;
}
