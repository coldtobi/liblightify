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

// Logging support

#include "liblightify-private.h"

#include "log.h"
#include "context.h"

#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <syslog.h>
#include <ctype.h>
#include <string.h>

void lightify_log(struct lightify_ctx *ctx,
           int priority, const char *file, int line, const char *fn,
           const char *format, ...)
{
        va_list args;
        va_start(args, format);
        ctx->log_fn(ctx, priority, file, line, fn, format, args);
        va_end(args);
}

void log_stderr(struct lightify_ctx *ctx,
                       int priority, const char *file, int line, const char *fn,
                       const char *format, va_list args)
{
        fprintf(stderr, "liblightify: %s: ", fn);
        vfprintf(stderr, format, args);
}

int log_priority(const char *priority)
{
        char *endptr;
        int prio;

        prio = strtol(priority, &endptr, 10);
        if (endptr[0] == '\0' || isspace(endptr[0]))
                return prio;
        if (strncmp(priority, "err", 3) == 0)
                return LOG_ERR;
        if (strncmp(priority, "info", 4) == 0)
                return LOG_INFO;
        if (strncmp(priority, "debug", 5) == 0)
                return LOG_DEBUG;
        return 0;
}

/**
 * lightify_set_log_fn:
 * @ctx: library context
 * @log_fn: function to be called for logging messages
 *
 * The built-in logging writes to stderr. It can be
 * overridden by a custom function, to plug log messages
 * into the user's logging functionality.
 *
 **/
LIGHTIFY_EXPORT int lightify_set_log_fn(struct lightify_ctx *ctx,
		void (*log_fn)(struct lightify_ctx *ctx, int priority, const char *file,
				int line, const char *fn, const char *format, va_list args))
{
	if (!ctx) return -EINVAL;
	ctx->log_fn = log_fn;
	info(ctx, "custom logging function %p registered\n", log_fn);
	return 0;
}

/**
 * lightify_get_log_priority:
 * @ctx: library context
 *
 * Returns: the current logging priority
 **/
LIGHTIFY_EXPORT int lightify_get_log_priority(struct lightify_ctx *ctx)
{
		if (!ctx) return -EINVAL;
        return ctx->log_priority;
}

/**
 * lightify_set_log_priority:
 * @ctx: library context
 * @priority: the new logging priority
 *
 * Set the current logging priority. The value controls which messages
 * are logged.
 **/
LIGHTIFY_EXPORT int lightify_set_log_priority(struct lightify_ctx *ctx, int priority)
{
		if (!ctx) return -EINVAL;
        ctx->log_priority = priority;
        return 0;
}
