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
 * node.h
 *
 *  Created on: 13.08.2015
 *      Author: tobi
 */

#ifndef SRC_NODE_H_
#define SRC_NODE_H_


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdint.h>

struct lightify_node;

// IMPORTANT NOTE //
// THIS API WILL ONLY MODIFY THE CACHED DATA -- They do NOT query the actual hardware.

/** Create new node entry and attach it to the ctx
 *
 * @param ctx
 * @param newnode
 * @return
 */
int lightify_node_new(struct lightify_ctx *ctx, struct lightify_node** newnode);

/** Remove a node from its context and free its memory.
 *
 * @param node to be manipulated
 * @return 0 on success, <0 on errors, like ENOMEM, EINVAL
 */
int lightify_node_remove(struct lightify_node* node);


/** Get the next node in the linked list
 *
 * @param node
 * @return next node or NULL
 */
struct lightify_node* lightify_node_get_nextnode(struct lightify_node *node);

/** Get the previous node in the linked list
 *
 * @param node
 * @return next node or NULL
 */
struct lightify_node* lightify_node_get_prevnode(struct lightify_node *node);

/** Gives a node a new name
 *
 * @param node to be manipulated
 * @param name to be set, NULL to clear the name.
 * @return 0 on success, <0 on errors, like EINVAL
 *
 * A copy of the name will be allocated.
 *
 */
int lightify_node_set_name(struct lightify_node* node, char *name);


/** Set a new node address (MAC adress)
 *
 * @param node
 * @param adr
 * @return
 *
 * \note Only the cache's data will be modified, there is no way to modify
 * the actual nodes' address.
 *
 */
int lightify_node_set_nodeadr(struct lightify_node* node, uint64_t adr);

/** Set the node's zone address
 *
 * @param node
 * @param adr
 * @return
 *
 * \note Only the cache's data will be modified, there is no way to modify
 * the actual zones' address.
 */
int lightify_node_set_zoneadr(struct lightify_node* node, uint16_t adr);


/** Set the node's zone address
 *
 * @param node
 * @param adr
 * @return
 *
 * \note Only the cache's data will be modified, there is no way to modify
 * the actual group address.
 */
int lightify_node_set_grpadr(struct lightify_node* node, uint16_t adr);


/** Set node's ZLL lamp types
 *
 * @param node
 * @param caps
 * @return
 */
int lightify_node_set_lamptype(struct lightify_node* node, enum lightify_node_type);


/** Set the color component of the node
 *
 * @param node
 * @param red
 * @return
 */
int lightify_node_set_red(struct lightify_node* node, int red);


/** Set the color component of the node
 *
 * @param node
 * @param red
 * @return
 */
int lightify_node_set_green(struct lightify_node* node, int green);


/** Set the color component of the node
 *
 * @param node
 * @return the value normalized from 0 to 255. negative numbers means: information not available.
 */
int lightify_node_set_blue(struct lightify_node* node, int blue);


/** Set the color component of the node
 *
 * @param node
 * @return the value normalized from 0 to 255. negative numbers means: information not available.
 */
int lightify_node_set_white(struct lightify_node* node, int white);


/** Set the Correlated Color Temperature
 *
 * @param node
 * @param cct -- normalized from 0 - 255 (negative values: not available)
 * @return negative on error
 */
int lightify_node_set_cct(struct lightify_node* node, int cct);

/** Set a new brightness value
 *
 * @param node
 * @param brightness
 * @return negative on error
 */
int lightify_node_set_brightness(struct lightify_node* node, int brightness);

/** Set the ON/OFF status of the node
 *
 * @param node
 * @param on 	0 means off, 1 means on (negative numbers are for "unknown")
 * @return negative on error
 */
int lightify_node_set_onoff(struct lightify_node* node, uint8_t on);

/** Set the online status
 *
 * @param node
 * @param state
 * @return negative on error
 */
int lightify_node_set_online_status(struct lightify_node* node, uint8_t state);

/** Set stale information
 *
 * @param node
 * @param stale 0 active, 1 stale
 * @return negative on error
 */
int lightify_node_set_stale(struct lightify_node *node, int stale);

/** Set the node's firmware version.
 *
 * The nodes' firmware version format seems to be
 * mayor.minor.maint.build, eg. 1.2.4.14, shown as 01020414
 * We're concatenate that into one unsigned long with this format:
 * 0xAABBCCDD -- AA = mayor, BB=minor, CC=maint, DD=build
 *
 * @param node to be manipulated
 * @param mayor version number
 * @param minor version number
 * @param maint (maintainance) version number
 * @param build number
 * @return negative on error
 */
int lightify_node_set_fwversion(struct lightify_node *node, uint8_t mayor,
		uint8_t minor, uint8_t maint, uint8_t build);

#endif /* SRC_NODE_H_ */
