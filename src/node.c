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

#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define MAX_NODE_NANE_LEN (16)

/** Information about the nodes (e.g. lamps)
 * kind of cache, will reflet state when last queried. */
struct lightify_node {

	/** pointer to the context */
	struct lightify_ctx *ctx;

	/* linked list */
	struct lightify_node *next;
	struct lightify_node *prev;

	/* node address and groups */
	uint64_t node_address;  /**< MAC of node */
	uint16_t zone_address;	/**< Zone address (16 bit short ZLL address)*/
	uint16_t group_address;	/**< Group address */

	/** lamp type */
	enum lightify_node_type node_type;

	/** name -- 16 bytes max, but we'll allocate memory for it. */
	char *name;

	int red; 	/**< red value */
	int green; 	/**< green value */
	int blue; 	/**< blue value */
	int white; 	/**< white value */
	int cct; 	/**< CCT */

	int brightness; /**< brightness */

	int is_on; /**< lamp on/off status. (0=off, 1=on, -1=unknown) */
	enum lightify_node_online_state online_status;

	/** what we  believe is the firmware version
	 * stuffed into one long -- 4 bytes: mayor.minor.maint.build */
	unsigned long fwversion;

	/** do we have confidence in the node status?
	 * Set to 1 if we did not get updated information on this node or a command
	 */
	int is_stale;
};

int lightify_node_new(struct lightify_ctx *ctx, struct lightify_node** newnode) {

	struct lightify_node *n;
	struct lightify_node *m;

	if (!ctx) return -EINVAL;

	n = calloc(1,sizeof(struct lightify_node));

	if (!n) return -ENOMEM;

	*newnode = n;

	n->red = -1;
	n->green = -1;
	n->blue = -1;
	n->white = -1;
	n->cct = -1;
	n->brightness = -1;
	n->is_on = -1;
	n->online_status = -1;

	n->ctx = ctx;
	m = ctx->nodes;

	if (!m) {
		ctx->nodes = n;
		return 0;
	}

	while(m->next) m=m->next;
	n->prev = m;
	m->next = n;

	return 0;
}

int lightify_node_remove(struct lightify_node* node) {

	if (!node) return -EINVAL;

	struct lightify_node *next = node->next;
	struct lightify_node *prev = node->prev;

	if (prev) {
		prev->next = next;
	} else {
		// first node
		node->ctx->nodes=next;
	}

	if (next) next->prev = prev;

	if (node->name) free(node->name);

	free(node);

	return 0;
}

struct lightify_node* lightify_node_get_nextnode(struct lightify_node *node) {
	if (!node) return NULL;
	return node->next;
}

struct lightify_node* lightify_node_get_prevnode(struct lightify_node *node) {
	if (!node) return NULL;
	return node->prev;
}


int lightify_node_set_name(struct lightify_node* node, char *name) {
	if (!node) return -EINVAL;

	if (node->name) free(node->name);
	node->name = NULL;

	if(name) {
		node->name = strndup(name, MAX_NODE_NANE_LEN);
		if (!node->name) return -ENOMEM;
	}
	return 0;
}

LIGHTIFY_EXPORT const char* lightify_node_get_name(struct lightify_node* node) {
	if (!node) return NULL;
	return node->name;
}

int lightify_node_set_nodeadr(struct lightify_node* node, uint64_t adr) {
	if(!node) return -EINVAL;
	node->node_address=adr;
	return 0;
}

LIGHTIFY_EXPORT uint64_t lightify_node_get_nodeadr(struct lightify_node* node) {
	if (!node) return 0;
	return node->node_address;
}

int lightify_node_set_zoneadr(struct lightify_node* node, uint16_t adr) {
	if(!node) return -EINVAL;
	node->zone_address=adr;
	return 0;
}

LIGHTIFY_EXPORT uint16_t lightify_node_get_zoneadr(struct lightify_node* node) {
	if (!node) return 0;
	return node->zone_address;
}

int lightify_node_set_grpadr(struct lightify_node* node, uint16_t adr) {
	if(!node) return -EINVAL;
	node->group_address=adr;
	return 0;
}

LIGHTIFY_EXPORT uint16_t lightify_node_get_grpadr(struct lightify_node* node) {
	if (!node) return 0;
	return node->group_address;
}

int lightify_node_set_lamptype(struct lightify_node* node, enum lightify_node_type type) {
	if(!node) return -EINVAL;
	node->node_type = type;
	return 0;
}

LIGHTIFY_EXPORT enum lightify_node_type lightify_node_get_lamptype(struct lightify_node* node) {
	if(!node) return -EINVAL;
	return node->node_type;
}

int lightify_node_set_red(struct lightify_node* node, int red) {
	if(!node) return -EINVAL;
	node->red = red;
	return 0;
}

LIGHTIFY_EXPORT int lightify_node_get_red(struct lightify_node* node) {
	if(!node) return -EINVAL;
	return node->red;
}

int lightify_node_set_blue(struct lightify_node* node, int blue) {
	if(!node) return -EINVAL;
	node->blue = blue;
	return 0;
}

LIGHTIFY_EXPORT int lightify_node_get_blue(struct lightify_node* node) {
	if(!node) return -EINVAL;
	return node->blue;
}

int lightify_node_set_green(struct lightify_node* node, int green) {
	if(!node) return -EINVAL;
	node->green = green;
	return 0;
}

LIGHTIFY_EXPORT int lightify_node_get_green(struct lightify_node* node) {
	if(!node) return -EINVAL;
	return node->green;
}

int lightify_node_set_white(struct lightify_node* node, int white) {
	if(!node) return -EINVAL;
	node->white = white;
	return 0;
}

LIGHTIFY_EXPORT int lightify_node_get_white(struct lightify_node* node) {
	if(!node) return -EINVAL;
	return node->white;
}

int lightify_node_set_cct(struct lightify_node* node, int cct) {
	if(!node) return -EINVAL;
	node->cct = cct;
	return 0;
}

LIGHTIFY_EXPORT int lightify_node_get_cct(struct lightify_node* node) {
	if(!node) return -EINVAL;
	return node->cct;
}

int lightify_node_set_brightness(struct lightify_node* node, int brightness) {
	if(!node) return -EINVAL;
	node->brightness = brightness;
	return 0;
}

LIGHTIFY_EXPORT int lightify_node_get_brightness(struct lightify_node* node) {
	if(!node) return -EINVAL;
	return node->brightness;
}


int lightify_node_set_onoff(struct lightify_node* node, uint8_t on) {
	if (!node) return -EINVAL;
	node->is_on= on;
	return 0;
}

LIGHTIFY_EXPORT int lightify_node_is_on(struct lightify_node* node) {
	if(!node) return -EINVAL;
	return node->is_on;
}

int lightify_node_set_online_status(struct lightify_node* node, uint8_t state) {
	if (!node) return -EINVAL;
	node->online_status= state;
	return 0;
}

LIGHTIFY_EXPORT int lightify_node_get_onlinestate(struct lightify_node* node) {
	if(!node) return -EINVAL;
	return node->online_status;
}

LIGHTIFY_EXPORT int lightify_node_is_stale(struct lightify_node *node) {
	if(!node) return -EINVAL;
	return node->is_stale;
}

int lightify_node_set_stale(struct lightify_node *node, int stale) {
	if(!node) return -EINVAL;
	node->is_stale = stale;
	return 0;
}

LIGHTIFY_EXPORT unsigned long lightify_node_get_fwversion(struct lightify_node *node) {
	if(!node) return 0;
	return node->fwversion;
}

int lightify_node_set_fwversion(struct lightify_node *node, uint8_t mayor, uint8_t minor, uint8_t maint, uint8_t build) {
	if(!node) return -EINVAL;
	unsigned long version = mayor << 24U | minor << 16U | maint << 8U | build;
	node->fwversion = version;
	return 0;
}
