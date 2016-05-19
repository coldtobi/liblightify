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

/*
 * libcontext.h
 *
 *  Created on: 13.08.2015
 *      Author: tobi
 */

#ifndef SRC_LIBCONTEXT_H_
#define SRC_LIBCONTEXT_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdarg.h>

/* Protocol versions */
#define GW_PROT_OLD  (0)
/* seen dec 2015 */
#define GW_PROT_1512 (0x1512)

/**
 * SECTION:liblightify
 * @short_description: liblightify context
 *
 * The context contains the default values for the library user,
 * and is passed to all library operations.
 */

struct lightify_nodes;
/**
 * lightify_ctx:
 *
 * Opaque object representing the library context.
 */
struct lightify_ctx {
	// Function pointer to the custom logging function.
	void (*log_fn)(struct lightify_ctx *ctx, int priority, const char *file,
			int line, const char *fn, const char *format, va_list args);

	/// Function pointer to the I/O handling -- read from
	int (*socket_read_fn)(struct lightify_ctx *ctx, unsigned char *msg, size_t size);

	/// Function pointer to the I/O handling -- read from
	int (*socket_write_fn)(struct lightify_ctx *ctx, unsigned char *msg, size_t size);

	/// Function pointer to the I/O handling -- write to

	// user supplied data, not used by the library
	void *userdata;

	int log_priority;

	/** fd for network socket*/
	int socket;

	/** pointer to the first node, if any. */
	struct lightify_node *nodes;

	/** pointer to the first group, if any */
	struct lightify_group *groups;

	/** request id counter*/
	uint32_t cnt;

	/** timeout for IO */
	struct timeval iotimeout;

	/** detected protocol variant */
	int gw_protocol_version;

};

#endif /* SRC_LIBCONTEXT_H_ */
