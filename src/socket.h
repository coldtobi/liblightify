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

/** \file socket.h
 *
 *  Created on: 17.08.2015
 *      Author: tobi
 */

#ifndef SRC_SOCKET_H_
#define SRC_SOCKET_H_

#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/** Write msg to socket; handling async IO and co
 *
 * @param ctx	library context
 * @param msg   what to write
 * @param size  how many bytes to be written
 * @return negative: error, positive: number of bytes written.
 */
int write_to_socket(struct lightify_ctx *ctx, unsigned char *msg, size_t size);

/** Read from socket, handling async I/O and co
 *
 * @param ctx 	library context
 * @param msg	where to store the result
 * @param size	expected read, also buffer size of msg.
 * @return actual read bytes, <0 on errors
 *
*/
int read_from_socket(struct lightify_ctx *ctx, unsigned char *msg, size_t size);

#endif /* SRC_SOCKET_H_ */
