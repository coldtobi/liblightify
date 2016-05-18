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

#ifndef _LIBLIGHTIFY_PRIVATE_H_
#define _LIBLIGHTIFY_PRIVATE_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <syslog.h>

#include <liblightify/liblightify.h>

static inline void __attribute__((always_inline, format(printf, 2, 3)))
lightify_log_null(struct lightify_ctx *ctx, const char *format, ...) {}

#define lightify_log_cond(ctx, prio, arg...) \
  do { \
    if (lightify_get_log_priority(ctx) >= prio) \
      lightify_log(ctx, prio, __FILE__, __LINE__, __FUNCTION__, ## arg); \
  } while (0)


#ifdef ENABLE_LOGGING
#  ifdef ENABLE_DEBUG
#    define dbg(ctx, arg...) lightify_log_cond(ctx, LOG_DEBUG, ## arg)
#  else
#    define dbg(ctx, arg...) lightify_log_null(ctx, ## arg)
#  endif
#  ifdef ENABLE_DEBUG_PROTO
#    define dbg_proto(ctx,arg...) lightify_log_cond(ctx, LOG_DEBUG, ## arg)
#  else
#    define dbg_proto(ctx,arg...) lightify_log_null(ctx, ## arg)
#  endif
#  define info(ctx, arg...) lightify_log_cond(ctx, LOG_INFO, ## arg)
#  define err(ctx, arg...) lightify_log_cond(ctx, LOG_ERR, ## arg)
#else
#  define dbg(ctx, arg...) lightify_log_null(ctx, ## arg)
#  define info(ctx, arg...) lightify_log_null(ctx, ## arg)
#  define err(ctx, arg...) lightify_log_null(ctx, ## arg)
#  define dbg_proto(ctx,arg...) lightify_log_null(ctx, ## arg)
#endif

#ifndef HAVE_SECURE_GETENV
#  ifdef HAVE___SECURE_GETENV
#    define secure_getenv __secure_getenv
#  else
#    warning neither secure_getenv nor __secure_getenv is available. Setting logging verbosity via enviorement is not supported.
#  endif
#endif

#define LIGHTIFY_EXPORT __attribute__ ((visibility("default")))

#endif
