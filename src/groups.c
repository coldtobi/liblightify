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

#include "liblightify-private.h"
#include "node.h"
#include "context.h"
#include "groups.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

/** \file groups.c
 *
 * Group support.
 *
 * Heavily Work in Progess - do not assume API stability!
 *
 */


struct lightify_group {
	struct lightify_ctx *ctx;

	// Linked list
    struct lightify_group *next;
	struct lightify_group *prev;

	/** Group ID */
	int id;

	/** Group name*/
	char *name;

};

int lightify_group_new(struct lightify_ctx *ctx, struct lightify_group **newgroup) {

	struct lightify_group *g, *ctx_g;
	if (!ctx)
		return -EINVAL;

	g = calloc(1, sizeof(struct lightify_group));
	if (!g)
		return -ENOMEM;

	ctx_g = ctx->groups;
	if (!ctx_g) {
		ctx->groups = g;
	} else {
		while (ctx_g->next)
			ctx_g = ctx_g->next;
		ctx_g->next = g;
		g->prev = ctx_g;
	}
	*newgroup = g;
	return 0;
}

int lightify_group_remove(struct lightify_group *grp) {
	if (!grp) return -EINVAL;

	struct lightify_group *next = grp->next;
	struct lightify_group *prev = grp->prev;

	if (prev) {
		prev->next = next;
	} else {
		// first grp
		grp->ctx->groups=next;
	}

	if (next) next->prev = prev;

	if (grp->name) free(grp->name);

	free(grp);

	return 0;
}

int lightify_group_set_name(struct lightify_group *grp, unsigned char *name) {
	if (!grp || !name) {
		return -EINVAL;
	}

	grp->name = strndup(name, 17);
	if (!grp->name) {
		return -ENOMEM;
	}
	return 0;
}

LIGHTIFY_EXPORT  const char *lightify_group_get_name(struct lightify_group *grp) {
	if (!grp) return 0;
    return grp->name;
}

int lightify_group_set_id(struct lightify_group *grp, int id) {
	if(!grp) return -EINVAL;
	grp->id = id;
	return 0;
}

LIGHTIFY_EXPORT int lightify_group_get_id(struct lightify_group *grp) {
	if(!grp) return -EINVAL;
	return grp->id;
}

// #FIXME export and document
LIGHTIFY_EXPORT struct lightify_group *lightify_group_get_next(struct lightify_ctx *ctx, struct lightify_group *current) {
	if (!ctx) return NULL;
	if (!current) return ctx->groups;
	return current->next;
}

// #FIXME export and document
LIGHTIFY_EXPORT struct lightify_node *lightify_group_get_next_node(struct lightify_group *grp, struct lightify_node *lastnode) {
	if (!grp) return NULL;
	uint16_t grpmask = 1 << (grp->id);

	while ( (lastnode = lightify_node_get_next(grp->ctx, lastnode))) {
		if ( grpmask & lightify_node_get_grpadr(lastnode)) return lastnode;
	}
	return NULL;
}
