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

#ifndef _LIBlightify_H_
#define _LIBlightify_H_

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>


#ifdef __cplusplus
extern "C" {
#endif


/* NXP has a nice ZigBee Light Link guide declaring all those types
 * NOTE: Definitions for  On/Off Light and Dimmable Plug Unit are unknown,
 * those two ZLL classes are missing for completeness... */
/** Known lamp types. */
enum lightify_node_type {
	LIGHTIFY_ONOFF_PLUG,
	LIGHTIFY_DIMABLE_LIGHT,
	LIGHTIFY_COLOUR_LIGHT,
	LIGHTIFY_EXT_COLOUR_LIGHT,
	LIGHTIFY_CCT_LIGHT,
	LIGHTIFY_UNKNOWNTYPE = 0xFF00 // if you encounter this, please encourage people to provide details. The lamp type is in the lower bits.
};

/** online / offline information. */
enum lightify_node_online_state {
	LIGHTIFY_OFFLINE = 0,
	LIGHTIFY_ONLINE = 2,
};

/** lightyfy_ctx
 *
 * library user context.
 * Stores information about the system and the states
 *
 */
struct lightify_ctx;

/** callback to roll your own I/O: Writing
 *
 * if the default is overriden, this function is called whenever the lib wants
 * to talk to the gateway.
 *
 * @param ctx library context
 * @param msg what to write
 * @param size how much to write
 * @return return a negative number (preferable from errno.h) on error,
 *  otherwise return the actually amount of bytes written.
 */
typedef int (*write_to_socket_fn)(struct lightify_ctx *ctx, unsigned char *msg, size_t size);

/** callback to roll your own I/O: Writing
 *
 * if the default is overriden, this function is called whenever the lib wants
 * to read from the gateway.
 *
 * @param ctx library context
 * @param msg where to place the received bytes
 * @param size how much to read. Do not read more than this!
 * @return return a negative number (preferable from errno.h) on error,
 *  otherwise return the actually amount of bytes read.
 */
typedef int (*read_from_socket_fn)(struct lightify_ctx *ctx, unsigned char *msg, size_t size);


// Library context and setup
/**
 *  Create a new library context object
 * @param ctx where to store the pointer of the object
 * @param reserved The second parameter, a pointer, is reserved for future use.
 *    Until then, provide NULL.
 *
 * @return 0 on success, negative value on error.
 */
int lightify_new(struct lightify_ctx **ctx, void *reserved);

/** Free the context structure including all data objects
 * associated.
 *
 * @param ctx
 * @return 0 on success, negative on errors (e.g wrong parameter)
 */
int lightify_free(struct lightify_ctx *ctx);

// Socket handling setup
/** Setup callbacks for custom socket I/O
 *
 *  This function can be used to override default socket I/O, eg
 *  if you want to roll your own.
 *
 *	If one of the pointers are NULL, default handling will be reinstated,
 *	which is basically unix I/O using read(2) and write(2).
 *
 *	The functions
 *
 * @param
 * @param write_to_socket_fn function pointer for the write function to be used
 * @param read_from_socket function pointer for the read functoin to be used.
 * @return 0 on success, negative on error.
 */
int lightify_set_socket_fn(struct lightify_ctx *ctx,
		write_to_socket_fn fpw,	read_from_socket_fn fpr);


/** set the socket fd to be used for communication.
 *
 * The lib expects here a int lightify_skt_setiotimeout(struct lightify_ctx *ctx, struct timeval tv);
 * ready-to-use socket supplied
 * by the app. It is safe to use non-blocking I/O.
 * To unset the fd, pass -1.
 *
 * @param ctx contect
 * @param socket file descriptor to be used or -1 to unset
 * @return 0 on success, negative on error
 */
int lightify_skt_setfd(struct lightify_ctx *ctx, int socket);

/** get the socket fd to be used for communication.
 *
 * @param ctx
 * @return socket, or -1 if no socket was set
 */
int lightify_skt_getfd(struct lightify_ctx *ctx);

/** set timeout to be used for socket communication.
 *
 * @param ctx library context
 * @param tv  timout to be used
 * @return 0 on success, negative on errors.
 *
 * \note the timeout is only used if the socket is setup using
 * O_NONBLOCK.
 */
int lightify_skt_setiotimeout(struct lightify_ctx *ctx, struct timeval tv);

/** Get the current timeout set.
 *
 * @param ctx
 * @return timeval. if ctx was NULL, returns a timeval set to zero.
 */
struct timeval lightify_skt_getiotimeout(struct lightify_ctx *ctx);


// Gateway stuff
/** Ask the gateway to provide informations about attached nodes
 *
 * @param ctx context
 * @return >=0 on success, negative on errors.
 * The number returned equals to the number of nodes found.
 *
 * The library will query the gateway to submit all known nodes.
 *
 * Disappeared nodes (and ones with stale information that cannot be updated)
 * will be removed from the list.
 *
 * \warning This will invalidate all pointers to node structures!
 *
 * \note on errors it might be that already a few nodes have been successfully
 * parsed. This can be checked via the API to retrieve node pointers:
 * If
 *
 */
int lightify_scan_nodes(struct lightify_ctx *ctx);

/** Search node via its MAC address.
 *
 * Search node via its unique ZLL MAC Address.
 *
 * @param ctx Library context
 * @param mac MAC Adress of node (64 bit value, guaranteed to be unique)
 * @return NULL if not found, otherwise pointer to node.
 */
struct lightify_node *lightify_get_nodefrommac(struct lightify_ctx *ctx, uint64_t mac);

/** Returns the next node in the linked list
 *
 * @param ctx  library context
 * @param node get the next from this node; if NULL, returns the first node
 * @return NULL or pointer to the node
 *
 * \warning it is not checked if node is actually belonging to this ctx, but
 * this might change in the future
 */
struct lightify_node* lightify_get_next_node(struct lightify_ctx *ctx,
		struct lightify_node *node );

/** Returns the next node in the linked list
 *
 * @param ctx  library context
 * @param node get the previous from this node
 * @return NULL or pointer to the node
 *
 * \warning it is not checked if node is actually belonging to this ctx,
 * but that might change in the future.
 */
struct lightify_node* lightify_get_prev_node(struct lightify_ctx *ctx,
		struct lightify_node *node );


// Managment stuff

int lightify_set_log_fn(struct lightify_ctx *ctx,
		void (*log_fn)(struct lightify_ctx *ctx, int priority, const char *file,
				int line, const char *fn, const char *format, va_list args));

int lightify_get_log_priority(struct lightify_ctx *ctx);

int lightify_set_log_priority(struct lightify_ctx *ctx, int priority);

void *lightify_get_userdata(struct lightify_ctx *ctx);

int lightify_set_userdata(struct lightify_ctx *ctx, void *userdata);


// Node information query

/** Retrieve the node's name
 *
 * @param node
 * @return pointer to name or NULL
 *
 */
const char* lightify_node_get_name(struct lightify_node* node);

/** Get the node's address (MAC address)
 *
 * @param node
 * @return node address
 *
 */
uint64_t lightify_node_get_nodeadr(struct lightify_node* node);

/** Get the zone address (short ZLL address)
 *
 * @param node
 * @return node address
 *
 */
uint16_t lightify_node_get_zoneadr(struct lightify_node* node);

/** Get the node's group address (short ZLL address)
 *
 * @param node
 * @return node address
 *
 */
uint16_t lightify_node_get_grpadr(struct lightify_node* node);

/** Get node's ZLL lamp type
 *
 * @param node
 * @return
 */
enum lightify_node_type lightify_node_get_lamptype(struct lightify_node* node);

/** Get the color components of the node: RED
 *
 * @param node
 * @return the value normalized from 0 to 255. negative numbers means: information not available.
 */
int lightify_node_get_red(struct lightify_node* node);

/** Get the color component of the node: GREEN
 *
 * @param node
 * @param green
 * @return
 */
int lightify_node_get_green(struct lightify_node* node);

/** Get the color component of the node: BLUE
 *
 * @param node
 * @param blue
 * @return
 */
int lightify_node_get_blue(struct lightify_node* node);

/** Get the color component of the node: WHITE
 *
 * @param node
 * @param white
 * @return
 */
int lightify_node_get_white(struct lightify_node* white);

/** Get the Correlated Color Temperature
 *
 * @param node
 * @return the value. negative numbers means: information not available.
 */
int lightify_node_get_cct(struct lightify_node* node);

/** Get a brightness value
 *
 * @param node
 * @return the value. negative numbers means: information not available.
 */
int lightify_node_get_brightness(struct lightify_node* node);

/** Return the node status
 *
 * @param node
 * @return 0 = off, 1 = 0n, -1=unknown
 */
int lightify_node_is_on(struct lightify_node* node);

/** Returns if we believe the node is live or stale (e.g command failed when
 * modifying the state of it)
 *
 * @param node
 * @return negative on error, 0 if not stale, 1 if not seen on last scan.
 */
int lightify_node_is_stale(struct lightify_node *node);

/** Get the online status
 *
 * @param node
 * @return
 */
int lightify_node_get_onlinestate(struct lightify_node* node);

// Node manipulation API -- will talk to the node

/** Turn lamp on or off
 *
 * @param ctx library context
 * @param node node to address. If NULL, broadcast.
 * @param onoff 1 to turn on, 0 do turn off
 * @return negative on error, 0 on success
 */
int lightify_request_node_set_onoff(struct lightify_ctx *ctx, struct lightify_node *node, int onoff);

/** Set CCT on lamp with configurable time.
 *
 * @param ctx
 * @param node
 * @param cct color temperature. (note: not filtered, but usually between 2700 and 6500)
 * @param fadetime in 1/10 seconds. 0 is instant.
 * @return
 */
int lightify_request_node_set_cct(struct lightify_ctx *ctx, struct lightify_node *node, unsigned int cct, unsigned int fadetime);

/** Set RGBW values
 *
 * \note the color values are from 0...255
 *
 * @param ctx  context
 * @param node node to be manipulated
 * @param r red value
 * @param g green value
 * @param b blue value
 * @param w white value
 * @param fadetime time in 1/10 seconds to reach final values.
 * @return
 */
int lightify_request_node_set_rgbw(struct lightify_ctx *ctx,
		struct lightify_node *node, unsigned int r, unsigned int g,
		unsigned int b,unsigned int w,unsigned int fadetime);

/** Set brightness
 *
 * @param ctx context
 * @param node to be manuipulated
 * @param level 0..100
 * @param fadetime in 1/10 seconds
 * @return
 */
int lightify_request_node_set_brightness(struct lightify_ctx *ctx,
		struct lightify_node *node, unsigned int level, unsigned int fadetime);

/** Update node information cache
 *
 * This function queries the gateway about current node information and the
 * data stored in the node's struct will be updated with the information from it.
 * This will also reset stale status, if it was previously set and the command
 * executed successfully.
 *
 * @param ctx context
 * @param node node
 * @return 0 on success, negative on error
 */
int lightify_request_update_node(struct lightify_ctx *ctx, struct lightify_node *node);


/** opaque struct handling the groups
 *
*/
struct lightify_group;

#warning document me!
struct lightify_group *lightify_group_get_next_group(struct lightify_ctx *ctx, struct lightify_group *current);

char *lightify_group_get_name(struct lightify_group *grp);

int lightify_group_get_id(struct lightify_group *grp);

int lightify_request_scan_groups(struct lightify_ctx *ctx);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
