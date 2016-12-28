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


/** \defgroup API_CTX Library Context */
/** \defgroup API_IO I/O setup and related functions */
/** \defgroup API_SUPPORT Support Functions */
/** \defgroup API_CALLBACK Call-backs */
/** \defgroup API_NODE Node manipulation and state */
/** \defgroup API_NODE_CACHE Functions for cached node information */

/** \defgroup API_GROUP Group manipulation and state */

/** \mainpage API Documentation for liblightify
 *
 *  \section ll_CAPI C API Documentation
 *
 *  \subsection ll_CAPI_Sections Library Sections
 *
 *  The library API contains those main sections:
 *  - Library context and I/O setup: \ref API_CTX
 *  - Library support routines, e.g. logging \ref API_SUPPORT
 *  - Library callbacks (e.g I/O) \ref API_CALLBACK
 *  - Nodes (Lamp) related: \ref API_NODE
 *  - Group relatedNode manipulation and state: \ref API_GROUP
 *
 *  \subsections ll_CAPI_NodeCache Node Information Cache
 *
 *  Nodes (Lamps) are queried from the gateway using lightify_nodes_scan().
 *  The library will then cache the retrieved information in memory, and when
 *  using the API to retrieve the properties the cached information is returned.
 *
 *  That means, if the node is manipulated externally, the cache will still read
 *  the old value as it needs an explicit request to update the node's cache.
 *
 *  Updating the cache can be done via an repeated scan for the nodes
 *  via lightify_nodes_scan() or via an update request lightify_nodes_update().
 *
 *  Note that a new scan will invalidate all library node objects -- all node pointers
 *  will become invalid and replaced by a new copy.
 *
 *  The functions to obtain cached node information are documented here:
 *  \ref API_NODE_CACHE
 *
 *  \section ll_APICPP C++ API Documentation
 *
 *  The C++ API is a wrapper for the C-Library.
 *  See the classes \ref Lightify and \ref Lightify_Node.
 *
 */

/*** \file liblightify.h
 *
 * This header defines the public interface to the library.
 */


/* NXP has a nice ZigBee Light Link guide declaring all those types
 * NOTE: Definitions for  On/Off Light and Dimmable Plug Unit are unknown,
 * those two ZLL classes are missing for completeness... */

/** Known lamp types and what they can do.
 *
 * \ingroup API_NODE
*/
enum lightify_node_type {
	LIGHTIFY_ONOFF_PLUG,      /**< Only On/off capable lamp/device */
	LIGHTIFY_DIMABLE_LIGHT,   /**< Can only control brightness */
	LIGHTIFY_COLOUR_LIGHT,    /**< RGBW */
	LIGHTIFY_EXT_COLOUR_LIGHT,/**< Tuneable White and RGBW  */
	LIGHTIFY_CCT_LIGHT,       /**< Tuneable White */
	LIGHTIFY_4WAY_SWITCH,	  /**< 4 Way switch (reported by user) */
	LIGHTIFY_UNKNOWNTYPE = 0xFF00 /**< if you encounter this, please encourage people to provide details. The lamp type is in the lower bits.*/
};

/** Node online / offline information provided from the gateway
 *
 * \ingroup API_NODE
*/
enum lightify_node_online_state {
	LIGHTIFY_OFFLINE = 0, 	/**< offline */
	LIGHTIFY_ONLINE = 2,    /**< online */
};

/** lightyfy_ctx
 *
 * library user context.
 * Stores information about the system and the states.
 *
 * \note this is opaque on purpose. Only use the API to access it.
 * \ingroup API_CTX
 */
struct lightify_ctx;


/** lightify_color_loop_spec
 *
 * struct to define one entry of a color loop
 */
struct lightify_color_loop_spec {
	uint8_t delay;
	uint8_t hue;
	uint8_t saturation;
	uint8_t brightness;
} ;

/** callback to roll your own I/O: Writing
 *
 * if the default function is overriden, this function is called whenever the
 * library wants to talk to the gateway.
 *
 * @param ctx library context
 * @param msg what to write
 * @param size how much to write
 * @return return a negative number (preferable from errno.h) on error,
 *  otherwise return the actually amount of bytes written.
 *
 * \ingroup API_CALLBACK API_IO
 *
 * \sa Default implementation is write_to_socket in module socket.h
 * \sa lightify_set_socket_fn
 */
typedef int (*write_to_socket_fn)(struct lightify_ctx *ctx, unsigned char *msg, size_t size);

/** callback to roll your own I/O: Reading
 *
 * if the default is overriden, this function is called whenever the library
 * wants to read from the gateway.
 *
 * @param ctx library context
 * @param msg where to place the received bytes
 * @param size how much to read. Do not read more than this!
 * @return return a negative number (preferable from errno.h) on error,
 *  otherwise return the actually amount of bytes read.
 *
 * \ingroup API_CALLBACK API_IO
 *
 * \sa Default implementation is read_from_socket in module socket.h
 * \sa lightify_set_socket_fn
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
 *
 * \ingroup API_CTX
 */
int lightify_new(struct lightify_ctx **ctx, void *reserved);

/** Free the context structure including all data objects
 * associated.
 *
 * @param ctx
 * @return 0 on success, negative on errors (e.g wrong parameter)
 *
 * \ingroup API_CTX
 */
int lightify_free(struct lightify_ctx *ctx);

// Socket handling setup
/** Setup callbacks for custom socket I/O
 *
 *  This function can be used to override default socket I/O, eg
 *  if you want to roll your own.
 *
 *	If one of the pointers are NULL, default handling will be reinstated,
 *	which is basically unix I/O using read(2) and write(2). See write_to_socket()
 *	and read_from_socket() for the implementation.
 *
 * @param
 * @param write_to_socket_fn function pointer for the write function to be used
 * @param read_from_socket function pointer for the read functoin to be used.
 * @return 0 on success, negative on error.
 *
 * \ingroup API_IO API_CALLBACK
 *
 * \sa write_to_socket read_from_socket
 */
int lightify_set_socket_fn(struct lightify_ctx *ctx,
		write_to_socket_fn fpw,	read_from_socket_fn fpr);


/** set the socket fd to be used for communication.
 *
 * When using the default I/O functions read_from_socket() and write_to_socket(),
 * the lib expects a ready-to-use socket supplied by the application.
 * It is safe to use non-blocking I/O.
 *
 * To unset the fd, pass -1.
 *
 * @param ctx contect
 * @param socket file descriptor to be used or -1 to unset
 * @return 0 on success, negative on error
 *
 * \ingroup API_IO
 *
 * \sa lightify_set_socket_fn lightify_skt_getfd
 *
 */
int lightify_skt_setfd(struct lightify_ctx *ctx, int socket);

/** get the socket fd to be used for communication.
 *
 * @param ctx
 * @return socket, or -1 if no socket was set
 *
 * \ingroup API_IO
 *
 * \sa read_from_socket lightify_set_socket_fn
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
 *
 * \note used by the default I/O implementation, write_to_socket() and
 * read_from_socket(), see there.
 *
 * \ingroup API_IO
 *
 * \sa write_to_socket() read_from_socket()
 */
int lightify_skt_setiotimeout(struct lightify_ctx *ctx, struct timeval tv);

/** Get the current timeout set.
 *
 * @param ctx
 * @return timeval. if ctx was NULL, returns a timeval set to zero.
 *
 * \note used by the default I/O implementation, write_to_socket() and
 * read_from_socket(), see there.
 *
 * \ingroup API_IO
 */
struct timeval lightify_skt_getiotimeout(struct lightify_ctx *ctx);


/** Ask the gateway to provide informations about attached nodes
 *
 * The library will query the gateway to submit all known nodes.
 *
 * Disappeared nodes (and ones with stale information that cannot be updated)
 * will be removed from the list.
 *
 * @param ctx context
 * @return the number of nodes detected (>=0) on success, negative on errors.
 *
 * \note All previous supplied node pointers become invalid after this call.
 *
 * \note on errors it might be that already a few nodes have been successfully
 * parsed. This can be checked via the API to retrieve node pointers. If there
 * are some. the call partially succeeded.
 *
 * \ingroup API_NODE
 */
int lightify_node_request_scan(struct lightify_ctx *ctx);

/** Search node via its MAC address.
 *
 * Search node via its unique ZLL MAC Address.
 *
 * @param ctx Library context
 * @param mac MAC Adress of node (64 bit value, guaranteed to be unique)
 * @return NULL if not found, otherwise pointer to node.
 * \ingroup API_NODE
 */
struct lightify_node *lightify_node_get_from_mac(struct lightify_ctx *ctx, uint64_t mac);

/** Returns the next node in the linked list
 *
 * @param ctx  library context
 * @param node get the next from this node; if NULL, returns the first node
 * @return NULL or pointer to the node
 *
 * \warning it is not checked if node is actually belonging to this ctx, but
 * this might change in the future
 * \ingroup API_NODE
 */
struct lightify_node* lightify_node_get_next(struct lightify_ctx *ctx,
		struct lightify_node *node );

/** Returns the next node in the linked list
 *
 * @param ctx  library context
 * @param node get the previous from this node
 * @return NULL or pointer to the node
 *
 * \warning it is not checked if node is actually belonging to this ctx,
 * but that might change in the future.
 * \ingroup API_NODE
 */
struct lightify_node* lightify_node_get_previous(struct lightify_ctx *ctx,
		struct lightify_node *node );


// Managment stuff

/** Setup logging callback.
 *
 * @param ctx library context
 * @param log_fn function to be used for logging
 * @return negative on error, >=0 on success.
 *
 * \ingroup API_SUPPORT
 *
 * \sa lightify_set_log_fn for the default implementation
 */
int lightify_set_log_fn(struct lightify_ctx *ctx,
		void (*log_fn)(struct lightify_ctx *ctx, int priority, const char *file,
				int line, const char *fn, const char *format, va_list args));

/** Get logging priority
 *
 * @param ctx context
 * @return current logging priority
 *
 * \ingroup API_SUPPORT
 */
int lightify_get_log_priority(struct lightify_ctx *ctx);

/** Set logging priority
 *
 * @param ctx context
 * @param priority priotiry to be set.
 * @return >=0 on success
 *
 * \ingroup API_SUPPORT
 */
int lightify_set_log_priority(struct lightify_ctx *ctx, int priority);

/** Get the stored userdata
 *
 * @param ctx context
 * @return >=0 on success
 *
 * \sa lightify_set_userdata
 * \ingroup API_CTX
 */
void *lightify_get_userdata(struct lightify_ctx *ctx);

/** Store a pointer in the library context.
 *
 * This can be used for user-data to be associated with the context.
 *
 *
 * @param ctx	context
 * @param userdata pointer to be stored
 * @return >=0 on success
 *
 * \ingroup API_CTX
 * \sa lightify_get_userdata
 */
int lightify_set_userdata(struct lightify_ctx *ctx, void *userdata);


// Node information query

/** Retrieve the node's name
 *
 * @param node
 * @return pointer to name or NULL
 *
 * \ingroup API_NODE
 */
const char* lightify_node_get_name(struct lightify_node* node);

/** Get the node's address (MAC address)
 *
 * @param node
 * @return node address
 *
 * \ingroup API_NODE
 */
uint64_t lightify_node_get_nodeadr(struct lightify_node* node);

/** Get the zone address (short ZLL address)
 *
 * @param node
 * @return node address
 *
 * \ingroup API_NODE
 */
uint16_t lightify_node_get_zoneadr(struct lightify_node* node);

/** Get the node's group address
 *
 * \note The group adress is a bitmask, every bit correspondending to a group.
 * The Bit set equals to the Group's ID.
 *
 * @param node
 * @return node address
 *
 * \ingroup API_NODE
 */
uint16_t lightify_node_get_grpadr(struct lightify_node* node);

/** Get node's ZLL lamp type
 *
 * @param node
 * @return lamp type
 *
 * \sa lightify_node_type
 *
 * \ingroup API_NODE
 */
enum lightify_node_type lightify_node_get_lamptype(struct lightify_node* node);

/** Get the color components of the node: RED
 *
 * @param node lamp
 * @return the value normalized from 0 to 255. negative numbers means: information not available.
 *
 * \ingroup API_NODE
 */
int lightify_node_get_red(struct lightify_node* node);

/** Get the color component of the node: GREEN
 *
 * @param node lamp
 * @return the value normalized from 0 to 255. negative numbers means: information not available.
 *
 * \ingroup API_NODE
 *
 * \note this function returns cached data. Be sure to refresh the data before when required.
 */
int lightify_node_get_green(struct lightify_node* node);

/** Get the color component of the node: BLUE
 *
 * @param node lamp
 * @return the value normalized from 0 to 255. negative numbers means: information not available.
 *
 * \ingroup API_NODE
 *
 * \note this function returns cached data. Be sure to refresh the data before when required.
 */
int lightify_node_get_blue(struct lightify_node* node);

/** Get the color component of the node: WHITE
 *
 * @param node lamp
 * @return the value normalized from 0 to 255. negative numbers means: information not available.
 *
 * \ingroup API_NODE
 *
 * \note this function returns cached data. Be sure to refresh the data before when required.
 */
int lightify_node_get_white(struct lightify_node* node);

/** Get the Correlated Color Temperature
 *
 * @param node lamp
 * @return the value. Negative numbers means: information not available.
 *
 * \ingroup API_NODE
 *
 * \note this function returns cached data. Be sure to refresh the data before when required.
 */
int lightify_node_get_cct(struct lightify_node* node);

/** Get a brightness value
 *
 * @param node lamp to be queried
 * @return the value. negative numbers means: information not available.
 *
 * \ingroup API_NODE
 *
 * \note this function returns cached data. Be sure to refresh the data before when required.
 */
int lightify_node_get_brightness(struct lightify_node* node);

/** Return the node status
 *
 * @param node lamp
 * @return 0 = off, 1 = 0n, -1=unknown
 *
 * \ingroup API_NODE
 *
 * \note this function returns cached data. Be sure to refresh the data before when required.
 */
int lightify_node_is_on(struct lightify_node* node);

/** Check if we think that the cache is actual with lamp state.
 *
 * A lamp can become stale if a command manipulating its state failed.
 *
 * Staleness is reset after scanning for nodes or updating a node.
 *
 * @param node lamp
 * @return negative on error, 0 if not stale, 1 otherwise
 *
 * \ingroup API_NODE
 */
int lightify_node_is_stale(struct lightify_node *node);

/** Get the online status
 *
 * @param node lamp
 * @return negative on error, otherwise see enum lightify_node_online_state
 *
 * \ingroup API_NODE
 */
int lightify_node_get_onlinestate(struct lightify_node* node);


/** Get the node's firmware version.
 *
 * The nodes' firmware version format seems to be
 * mayor.minor.maint.build, eg. 1.2.4.14, shown as 01020414
 * We're concatenate that into one unsigned long with this format:
 * 0xAABBCCDD -- AA = mayor, BB=minor, CC=maint, DD=build
 *
 * @param node lamp
 * @return long with version, ZERO if node was NULL.
 *
 *  \ingroup API_NODE
 */
unsigned long lightify_node_get_fwversion(struct lightify_node *node);

// Node manipulation API -- will talk to the node

/** Turn lamp on or off
 *
 * @param ctx library context
 * @param node node to address. If NULL, broadcast.
 * @param onoff 1 to turn on, 0 do turn off
 * @return negative on error, >=0 on success
 *
 * \ingroup API_NODE
 */
int lightify_node_request_onoff(struct lightify_ctx *ctx, struct lightify_node *node, int onoff);

/** Set CCT on lamp with configurable time.
 *
 * @param ctx
 * @param node
 * @param cct color temperature. (note: not filtered, but usually between 2700 and 6500)
 * @param fadetime in 1/10 seconds. 0 is instant.
 * @return negative on error, >=0 on success
 *
 * \ingroup API_NODE
 */
int lightify_node_request_cct(struct lightify_ctx *ctx, struct lightify_node *node, unsigned int cct, unsigned int fadetime);

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
 * @return negative on error, >=0 on success
 *
 * \ingroup API_NODE
 */
int lightify_node_request_rgbw(struct lightify_ctx *ctx,
		struct lightify_node *node, unsigned int r, unsigned int g,
		unsigned int b,unsigned int w,unsigned int fadetime);

/** Set brightness
 *
 * @param ctx context
 * @param node to be manuipulated
 * @param level 0..100
 * @param fadetime in 1/10 seconds
 * @return negative on error, >=0 on success
 *
 * \ingroup API_NODE
 */
int lightify_node_request_brightness(struct lightify_ctx *ctx,
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
 * @return negative on error, >=0 on success
 *
 * \ingroup API_NODE
 */
int lightify_node_request_update(struct lightify_ctx *ctx, struct lightify_node *node);


/** opaque struct handling the groups
 *
 * \ingroup API_GROUP
*/
struct lightify_group;

/** Get next known group
 *
 * @param ctx context
 * @param current last group queried
 * @return next group in list or NULL if there isn't one
 *
 * \ingroup API_GROUP
 */
struct lightify_group *lightify_group_get_next(struct lightify_ctx *ctx, struct lightify_group *current);

/** Get previous known group
 *
 * @param ctx context
 * @param current last group queried
 * @return previous group in list or NULL if there isn't one
 *
 * \ingroup API_GROUP
 */
struct lightify_group *lightify_group_get_previous(struct lightify_ctx *ctx, struct lightify_group *current);

/** Get the name associated with the group
 *
 * @param grp Group pointer
 * @return pointer to a string with the name
 *
 * \ingroup API_GROUP
 */
const char *lightify_group_get_name(struct lightify_group *grp);

/** Get the ID of the group
 *
 * @param grp
 * @return group id or negative on error.
 *
 * \ingroup API_GROUP
 */
int lightify_group_get_id(struct lightify_group *grp);

/** Request the list of known groups
 *
 * @param ctx context
 * @return negative on error, else number of retrieved groups (might be zero)
 *
 * \ingroup API_GROUP
 */
int lightify_group_request_scan(struct lightify_ctx *ctx);

/** Get the next node ptr associated with the group
 *
 * @param grp group the node must be in
 * @param lastnode last node asked for, NULL if the first
 * @return NULL is not found, else pointer.
 *
 * \note you must scan for nodes to be able to associate nodes with the groups.
 *
 * \ingroup API_GROUP
 */
struct lightify_node *lightify_group_get_next_node(struct lightify_group *grp, struct lightify_node *lastnode);

/** Request group to be turned off or on
 *
 * @param ctx context
 * @param group group ptr
 * @param onoff on or off ( true or false)
 * @return >=0 on success. negative on error.
 *
 * \ingroup API_GROUP
 */
int lightify_group_request_onoff(struct lightify_ctx *ctx, struct lightify_group *group, int onoff);

/** Set group CCT
 *
 * @param ctx context
 * @param group group
 * @param cct CCT
 * @param fadetime time in 1/10 secs
 * @return >=0 on success. negative on error.
 *
 * \ingroup API_GROUP
 */
int lightify_group_request_cct(struct lightify_ctx *ctx, struct lightify_group *group, unsigned int cct, unsigned int fadetime);

/** Set RGBW values
 *
 * \note some lamps cannot set white, also white and rgb might be exclusive.
 *
 * @param ctx context
 * @param group
 * @param r red
 * @param g green
 * @param b blue
 * @param w white
 * @param fadetime
 * @return >=0 on success. negative on error.
 *
 * \ingroup API_GROUP
 */
int lightify_group_request_rgbw(struct lightify_ctx *ctx,
		struct lightify_group *group, unsigned int r, unsigned int g,
		unsigned int b,unsigned int w,unsigned int fadetime) ;

/** Set Group brightness
 *
 * @param ctx
 * @param group
 * @param level
 * @param fadetime
 * @return >=0 on success. negative on error.
 *
 * \ingroup API_GROUP
 */
int lightify_group_request_brightness(struct lightify_ctx *ctx,
		struct lightify_group *group, unsigned int level, unsigned int fadetime) ;

/** Request color loop
 *
 * Request the lamp to enter color loop mode.
 * It will stay in loop mode until other commands are issued.
 *
 * The color loop consists of a small programm, built from
 * lightify_color_loop_specs. You need to supply an array of those.
 *
 * The first entry in the array is special, it defines the starting point of
 * the loop. Please note that the delay field of this entry *must* be 0x3C.
 * it will not have any time-influencing properties.
 *
 * The static_bytes seems hard-coded but playing with it shows that they have
 * indeed an influence on the behavior. Supply NULL to get those or provide
 * your own 8 bytes.
 *
 * @param ctx
 * @param node
 * @param colorspec
 * @param number_of_specs how big is the loop. (Must be exactly 15 for the time being)
 * @param static_bytes 8-bytes to send as static bytes, instead of the hardcoded ones.
 *        (use NULL for those)
 *
 * @return negative on error, 0 on success.,
 * 		  (eg. EINVAL on parameter problems, including if you did not provide the 0x3C.
 * 		  Other codes might be returned as well.)
 *
 *  \sa lightify_color_loop_specs
 *
 *  \warning this API will change if we find out about the static bytes.
 *  (That's why were still at SO-NAME 0...)
 *
 *  \note "success" is only the report from the gateway. That does not mean it will
 *  really start looping -- sorry, no API known to query that.
 */
int lightify_node_request_color_loop(struct lightify_ctx *ctx,
		struct lightify_node *node,
		const struct lightify_color_loop_spec* colorspec,
		unsigned int number_of_specs, const uint8_t static_bytes[8]);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
