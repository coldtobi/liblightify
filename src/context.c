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
#include "context.h"
#include "log.h"
#include "node.h"

#include "socket.h"

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

enum msg_header {
	HEADER_LEN_LSB,
	HEADER_LEN_MSB,
	HEADER_FLAGS,
	HEADER_CMD,
	HEADER_REQ_ID_B0,
	HEADER_REQ_ID_B1,
	HEADER_REQ_ID_B2,
	HEADER_REQ_ID_B3,
	HEADER_PAYLOAD_START
};

enum msg_0x13_query {
	QUERY_0x13_REQTYPE = HEADER_PAYLOAD_START,
	QUERY_0x13_SIZE
};

enum msg_0x13_answer {
	ANSWER_0x13_UNKNOWN1 = HEADER_PAYLOAD_START,
	ANSWER_0x13_NODESCNT_LSB,
	ANSWER_0x13_NODESCNT_MSB,
	ANSWER_0x13_SIZE
};

enum msg_0x13_answer_node {
	ANSWER_0x13_NODE_ADR16_LSB,
	ANSWER_0x13_NODE_ADR16_MSB,
	ANSWER_0x13_NODE_ADR64_B0,
	ANSWER_0x13_NODE_ADR64_B1,
	ANSWER_0x13_NODE_ADR64_B2,
	ANSWER_0x13_NODE_ADR64_B3,
	ANSWER_0x13_NODE_ADR64_B4,
	ANSWER_0x13_NODE_ADR64_B5,
	ANSWER_0x13_NODE_ADR64_B6,
	ANSWER_0x13_NODE_ADR64_B7,
	ANSWER_0x13_NODE_NODETYPE,
	ANSWER_0x13_UNKNOWN2,
	ANSWER_0x13_UNKNOWN3,
	ANSWER_0x13_UNKNOWN4,
	ANSWER_0x13_UNKNOWN5,
	ANSWER_0x13_NODE_ONLINE_STATE,
	ANSWER_0x13_NODE_GRP_MEMBER_LSB,
	ANSWER_0x13_NODE_GRP_MEMBER_MSB,
	ANSWER_0x13_NODE_ONOFF_STATE,
	ANSWER_0x13_NODE_DIM_LEVEL,
	ANSWER_0x13_NODE_CCT_LSB,
	ANSWER_0x13_NODE_CCT_MSB,
	ANSWER_0x13_NODE_R,
	ANSWER_0x13_NODE_G,
	ANSWER_0x13_NODE_B,
	ANSWER_0x13_NODE_W,
	ANSWER_0x13_NODE_NAME_START,
	ANSWER_0x13_NODE_LENGTH = 42
};

enum msg_0x31_query {
	QUERY_0x31_NODEADR64_B0 = HEADER_PAYLOAD_START,
	QUERY_0x31_NODEADR64_B1,
	QUERY_0x31_NODEADR64_B2,
	QUERY_0x31_NODEADR64_B3,
	QUERY_0x31_NODEADR64_B4,
	QUERY_0x31_NODEADR64_B5,
	QUERY_0x31_NODEADR64_B6,
	QUERY_0x31_NODEADR64_B7,
	QUERY_0x31_LEVEL,
	QUERY_0x31_FADETIME_LSB,
	QUERY_0x31_FADETIME_MSB,
	QUERY_0x31_SIZE
};

enum msg_0x31_answer {
	ANSWER_0x31_STATE = HEADER_PAYLOAD_START,
	ANSWER_0x31_UNKNOWN1,
	ANSWER_0x31_UNKNOWN2,
	ANSWER_0x31_NODEADR64_B0,
	ANSWER_0x31_NODEADR64_B1,
	ANSWER_0x31_NODEADR64_B2,
	ANSWER_0x31_NODEADR64_B3,
	ANSWER_0x31_NODEADR64_B4,
	ANSWER_0x31_NODEADR64_B5,
	ANSWER_0x31_NODEADR64_B6,
	ANSWER_0x31_NODEADR64_B7,
	ANSWER_0x31_UNKNOWN3,
	ANSWER_0x31_SIZE
};

enum msg_0x32_query {
	QUERY_0x32_NODEADR64_B0 = HEADER_PAYLOAD_START,
	QUERY_0x32_NODEADR64_B1,
	QUERY_0x32_NODEADR64_B2,
	QUERY_0x32_NODEADR64_B3,
	QUERY_0x32_NODEADR64_B4,
	QUERY_0x32_NODEADR64_B5,
	QUERY_0x32_NODEADR64_B6,
	QUERY_0x32_NODEADR64_B7,
	QUERY_0x32_ONOFF,
	QUERY_0x32_SIZE
};

enum msg_0x32_answer {
	ANSWER_0x32_STATE = HEADER_PAYLOAD_START,
	ANSWER_0x32_RESERVED_LSB,
	ANSWER_0x32_RESERVED_MSB,
	ANSWER_0x32_NODEADR64_B0,
	ANSWER_0x32_NODEADR64_B1,
	ANSWER_0x32_NODEADR64_B2,
	ANSWER_0x32_NODEADR64_B3,
	ANSWER_0x32_NODEADR64_B4,
	ANSWER_0x32_NODEADR64_B5,
	ANSWER_0x32_NODEADR64_B6,
	ANSWER_0x32_NODEADR64_B7,
	ANSWER_0x32_UNKNOWN1,
	ANSWER_0x32_SIZE
};

enum msg_0x33_query {
	QUERY_0x33_NODEADR64_B0 = HEADER_PAYLOAD_START,
	QUERY_0x33_NODEADR64_B1,
	QUERY_0x33_NODEADR64_B2,
	QUERY_0x33_NODEADR64_B3,
	QUERY_0x33_NODEADR64_B4,
	QUERY_0x33_NODEADR64_B5,
	QUERY_0x33_NODEADR64_B6,
	QUERY_0x33_NODEADR64_B7,
	QUERY_0x33_CCT_LSB,
	QUERY_0x33_CCT_MSB,
	QUERY_0x33_FADETIME_LSB,
	QUERY_0x33_FADETIME_MSB,
	QUERY_0x33_SIZE
};

enum msg_0x33_answer {
	ANSWER_0x33_STATE = HEADER_PAYLOAD_START,
	ANSWER_0x33_RESERVED_LSB,
	ANSWER_0x33_RESERVED_MSB,
	ANSWER_0x33_NODEADR64_B0,
	ANSWER_0x33_NODEADR64_B1,
	ANSWER_0x33_NODEADR64_B2,
	ANSWER_0x33_NODEADR64_B3,
	ANSWER_0x33_NODEADR64_B4,
	ANSWER_0x33_NODEADR64_B5,
	ANSWER_0x33_NODEADR64_B6,
	ANSWER_0x33_NODEADR64_B7,
	ANSWER_0x33_UNKNOWN1,
	ANSWER_0x33_SIZE
};


enum msg_0x36_query {
	QUERY_0x36_NODEADR64_B0 = HEADER_PAYLOAD_START,
	QUERY_0x36_NODEADR64_B1,
	QUERY_0x36_NODEADR64_B2,
	QUERY_0x36_NODEADR64_B3,
	QUERY_0x36_NODEADR64_B4,
	QUERY_0x36_NODEADR64_B5,
	QUERY_0x36_NODEADR64_B6,
	QUERY_0x36_NODEADR64_B7,
	QUERY_0x36_R,
	QUERY_0x36_G,
	QUERY_0x36_B,
	QUERY_0x36_W,
	QUERY_0x36_FADETIME_LSB,
	QUERY_0x36_FADETIME_MSB,
	QUERY_0x36_SIZE
};

enum msg_0x36_answer {
	ANSWER_0x36_STATE = HEADER_PAYLOAD_START,
	ANSWER_0x36_RESERVED_LSB,
	ANSWER_0x36_RESERVED_MSB,
	ANSWER_0x36_NODEADR64_B0,
	ANSWER_0x36_NODEADR64_B1,
	ANSWER_0x36_NODEADR64_B2,
	ANSWER_0x36_NODEADR64_B3,
	ANSWER_0x36_NODEADR64_B4,
	ANSWER_0x36_NODEADR64_B5,
	ANSWER_0x36_NODEADR64_B6,
	ANSWER_0x36_NODEADR64_B7,
	ANSWER_0x36_UNKNOWN1,
	ANSWER_0x36_SIZE
};

enum msg_0x68_query {
	QUERY_0x68_NODEADR64_B0 = HEADER_PAYLOAD_START,
	QUERY_0x68_NODEADR64_B1,
	QUERY_0x68_NODEADR64_B2,
	QUERY_0x68_NODEADR64_B3,
	QUERY_0x68_NODEADR64_B4,
	QUERY_0x68_NODEADR64_B5,
	QUERY_0x68_NODEADR64_B6,
	QUERY_0x68_NODEADR64_B7,
	QUERY_0x68_SIZE
};

enum msg_0x68_answer {
	ANSWER_0x68_STATE = HEADER_PAYLOAD_START,
	ANSWER_0x68_NONODES_LSB,
	ANSWER_0x68_NONODES_MSB,
	ANSWER_0x68_NODEADR64_B0,
	ANSWER_0x68_NODEADR64_B1,
	ANSWER_0x68_NODEADR64_B2,
	ANSWER_0x68_NODEADR64_B3,
	ANSWER_0x68_NODEADR64_B4,
	ANSWER_0x68_NODEADR64_B5,
	ANSWER_0x68_NODEADR64_B6,
	ANSWER_0x68_NODEADR64_B7,
	ANSWER_0x68_UNKNOWN1,
	ANSWER_0x68_ONLINESTATE,
	ANSWER_0x68_ONOFF,
	ANSWER_0x68_DIM_LEVEL,
	ANSWER_0x68_CCT_LSB,
	ANSWER_0x68_CCT_MSB,
	ANSWER_0x68_R,
	ANSWER_0x68_G,
	ANSWER_0x68_B,
	ANSWER_0x68_W,
	ANSWER_0x68_SIZE
};

// 0 seems success, non-zero error.
static int decode_status(unsigned char code) {
	switch (code) {
		// success
		case 0x00: return 0;
		case 0x15: return ENODEV;
		default: return EIO;
	}
}

/** Find node via mac address (from cache)
 *
 * @param ctx  context
 * @param mac  64bit mac adress
 * @return pointer to node or NULL.
 */
LIGHTIFY_EXPORT struct lightify_node *lightify_get_nodefrommac(struct lightify_ctx *ctx, uint64_t mac) {
	if (!ctx) return NULL;

	struct lightify_node *ret = ctx->nodes;
	while(ret) {
		if (lightify_node_get_nodeadr(ret) == mac) return ret;
		ret = lightify_node_get_nextnode(ret);
	}
	return NULL;
}

/** Helper function to assemble a uint64_t from a message buffer
 *
 * @param msg
 * @return value
 */
static uint64_t uint64_from_msg(uint8_t *msg) {
	uint64_t tmp;
	tmp =  msg[7]; tmp <<=8;
	tmp |= msg[6]; tmp <<=8;
	tmp |= msg[5]; tmp <<=8;
	tmp |= msg[4]; tmp <<=8;
	tmp |= msg[3]; tmp <<=8;
	tmp |= msg[2]; tmp <<=8;
	tmp |= msg[1]; tmp <<=8;
	tmp |= msg[0];
	return tmp;
}

static void msg_from_uint64(unsigned char *pmsg, uint64_t mac) {
	*pmsg++ = mac & 0xff;
	*pmsg++ = mac >> 8 & 0xff;
	*pmsg++ = mac >> 16 & 0xff;
	*pmsg++ = mac >> 24 & 0xff;
	*pmsg++ = mac >> 32 & 0xff;
	*pmsg++ = mac >> 40 & 0xff;
	*pmsg++ = mac >> 48 & 0xff;
	*pmsg++ = mac >> 56 & 0xff;
}

/** Helper function to assemble a uint16_t from a message buffer
 *
 * @param msg
 * @return value
 */
static uint16_t uint16_from_msg(uint8_t *msg) {
	uint16_t tmp;
	tmp = msg[0] | (msg[1]<<8);
	return tmp;
}


/** helper to fill telegram header
 *
 * @param msg message buffer to be filled (at least HEADER_PAYLOAD_START bytes long)
 * @param len telegram len. Will be adjusted by the header size.
 * @param token to be used in token field. Use ++ctx->cnt but keep a copy to compare.
 * @param flags message flags.
 * @param command to be put into the command field.
 *
 */
static void fill_telegram_header(unsigned char *msg, unsigned int len, unsigned long token, unsigned char flags, unsigned char command)
{
	len-=2;
	msg[HEADER_LEN_LSB] = len & 0xff;
	msg[HEADER_LEN_MSB] = len >> 8;
	msg[HEADER_FLAGS] = flags;
	msg[HEADER_CMD] = command;
	msg[HEADER_REQ_ID_B0] = token & 0xff;
	msg[HEADER_REQ_ID_B1] = token >> 8 & 0xff;
	msg[HEADER_REQ_ID_B2] = token >> 16 & 0xff;
	msg[HEADER_REQ_ID_B3] = token >> 24 & 0xff;
}


static int check_header_response(unsigned char *msg, unsigned long token,
		unsigned char cmd) {

	unsigned long token2;
	/* check the header if plausible */
	/* check if the token we've supplied is also the returned one. */
	token2 = msg[HEADER_REQ_ID_B0] | (msg[HEADER_REQ_ID_B1] << 8) |
			 (msg[HEADER_REQ_ID_B1] << 16) | (msg[HEADER_REQ_ID_B1] << 24);
	if (token != token2) return -EPROTO;
	if (msg[HEADER_CMD] != cmd) return -EPROTO;
	return 0;
}

LIGHTIFY_EXPORT struct lightify_node* lightify_get_next_node(struct lightify_ctx *ctx,
		struct lightify_node *node ) {

	if(!ctx) return NULL;
	if(node) return lightify_node_get_nextnode(node);
	return ctx->nodes;
}

LIGHTIFY_EXPORT struct lightify_node* lightify_get_prev_node(struct lightify_ctx *ctx,
		struct lightify_node *node )
{
	if(!ctx) return NULL;
	if(!node) return NULL;
    return lightify_node_get_prevnode(node);
}

LIGHTIFY_EXPORT void *lightify_get_userdata(struct lightify_ctx *ctx)
{
        if (!ctx) return NULL;
        return ctx->userdata;
}

LIGHTIFY_EXPORT int lightify_set_userdata(struct lightify_ctx *ctx, void *userdata)
{
        if (!ctx) return -EINVAL;
        ctx->userdata = userdata;
        return 0;
}

LIGHTIFY_EXPORT int lightify_set_socket_fn(struct lightify_ctx *ctx,
		write_to_socket_fn fpw, read_from_socket_fn fpr) {

	if (!ctx) return -EINVAL;
	if (!fpw || !fpr) {;
		ctx->socket_read_fn = read_from_socket;
		ctx->socket_write_fn = write_to_socket;
		return 0;
	}

	ctx->socket_read_fn = fpr;
	ctx->socket_write_fn = fpw;
	return 0;
}

LIGHTIFY_EXPORT int lightify_new(struct lightify_ctx **ctx, void *reserved)
{
        struct lightify_ctx *c;

        c = calloc(1, sizeof(struct lightify_ctx));
        if (!c) return -ENOMEM;

        c->log_fn = log_stderr;
        c->log_priority = LOG_ERR;

#ifdef HAVE_SECURE_GETENV
        /* environment overwrites config */
        const char *env;
        env = secure_getenv("lightify_LOG");
        if (env != NULL)
                lightify_set_log_priority(c, log_priority(env));
#endif

        info(c, "ctx %p created\n", c);
        dbg(c, "log_priority=%d\n", c->log_priority);
        *ctx = c;

        c->socket = -1;
        c->iotimeout.tv_sec=1;

		c->socket_read_fn = read_from_socket;
		c->socket_write_fn = write_to_socket;

        return 0;
}

LIGHTIFY_EXPORT int lightify_free(struct lightify_ctx *ctx) {
	if (!ctx) return -EINVAL;

	while(ctx->nodes) {
		dbg(ctx, "freeing node %p.\n", ctx->nodes);
		lightify_node_remove(ctx->nodes);
	}
	dbg(ctx, "context %p freed.\n", ctx);
	free(ctx);
	return 0;
}

LIGHTIFY_EXPORT int lightify_scan_nodes(struct lightify_ctx *ctx) {
	int n,m;
	int no_of_nodes;
	long token;

	if (!ctx) return -EINVAL;
	if (!ctx->socket_read_fn && !ctx->socket_write_fn && ctx->socket == -1) return -EBADF;

	/* remove old node information */
	struct lightify_node *node = ctx->nodes;
	while ( (node = lightify_get_next_node(ctx, NULL))) {
		dbg(ctx, "freeing node %p.\n", node);
		lightify_node_remove(node);
	}

	token = ++ctx->cnt;

	/* to avoid problems with packing, we need to use a char array.
	 * to assist we'll have this fine enum */
	uint8_t msg[42];

	/* 0x13 command to get all node's informations. */
	fill_telegram_header(msg, QUERY_0x13_SIZE, token, 0x00, 0x13);
	msg[QUERY_0x13_REQTYPE] = 0x01;

	n = ctx->socket_write_fn(ctx, msg, QUERY_0x13_SIZE);
	if ( n < 0 ) {
		info(ctx,"socket_write_fn error %d\n", n);
		return n;
	}
	if ( n != QUERY_0x13_SIZE) {
		info(ctx,"short write %d!=%d\n", QUERY_0x13_SIZE, n);
		return -EIO;
	}

	/* read the header */
	n = ctx->socket_read_fn(ctx, msg, ANSWER_0x13_SIZE);
	if (n < 0) {
		info(ctx,"socket_read_fn error %d\n", n);
		return n;
	}
	if (n != ANSWER_0x13_SIZE) {
		info(ctx,"short read %d!=%d\n", ANSWER_0x13_SIZE, n);
		return -EIO;
	}

	/* check the header if plausible */
	/* check if the token we've supplied is also the returned one. */
	n = check_header_response(msg, token, 0x13);
	if ( n < 0 ) {
		info(ctx,"Invalid response (header)\n");
		return n;
	}

	/* check if the message length is as expected */
	no_of_nodes = msg[ANSWER_0x13_NODESCNT_LSB] | (msg[ANSWER_0x13_NODESCNT_MSB] <<8);
	m = msg[HEADER_LEN_LSB] | (msg[HEADER_LEN_MSB] << 8);
	/*info(ctx, "0x13: received %d bytes\n",m);*/
	if ( no_of_nodes * ANSWER_0x13_NODE_LENGTH + ANSWER_0x13_SIZE - 2 != m) {
		info(ctx, "Reponse len unexpected for %d nodes: %d!=%d.\n", no_of_nodes,
				no_of_nodes * ANSWER_0x13_NODE_LENGTH + ANSWER_0x13_SIZE - 2, m);
		return -EPROTO;
	}

	if (msg[HEADER_PAYLOAD_START]) {
		info(ctx, "strange byte at PAYLOAD_START: %d\n", msg[HEADER_PAYLOAD_START]);
	}

	/* read each node..*/
	while(no_of_nodes--) {
		node = NULL;
		uint64_t tmp64;
		n = ctx->socket_read_fn(ctx, msg, ANSWER_0x13_NODE_LENGTH);
		if (n< 0) return n;
		if (ANSWER_0x13_NODE_LENGTH != n ) {
			info(ctx,"read node info: short read %d!=%d\n", ANSWER_0x13_NODE_LENGTH, n);
			return -EIO;
		}

		n = lightify_node_new(ctx, &node);
		if (n < 0) {
			info(ctx, "create node error %d", n);
			return n;
		}
		tmp64 = uint64_from_msg(&msg[ANSWER_0x13_NODE_ADR64_B0]);
		lightify_node_set_nodeadr(node, tmp64);

		lightify_node_set_zoneadr(node, uint16_from_msg(&msg[ANSWER_0x13_NODE_ADR16_LSB]));
		lightify_node_set_grpadr(node, uint16_from_msg(&msg[ANSWER_0x13_NODE_GRP_MEMBER_LSB]));

		lightify_node_set_name(node, (char*) &msg[ANSWER_0x13_NODE_NAME_START]);
		info(ctx, "new node: %s\n", lightify_node_get_name(node));

		n = msg[ANSWER_0x13_NODE_NODETYPE];
		switch (n) {
		case 0x00 : // Plug
			lightify_node_set_lamptype(node, LIGHTIFY_ONOFF_PLUG);
			break;
		case 0x02 : // CCT light
			lightify_node_set_lamptype(node, LIGHTIFY_CCT_LIGHT);
			break;
		case 0x04 : // dimable
			lightify_node_set_lamptype(node, LIGHTIFY_DIMABLE_LIGHT);
			break;
		case 0x08 : // RGB
			lightify_node_set_lamptype(node, LIGHTIFY_COLOUR_LIGHT);
			break;
		case 0x0a : // CCT, RGB
			lightify_node_set_lamptype(node, LIGHTIFY_EXT_COLOUR_LIGHT);
			break;
		default: // maybe the missing dimmer plug or on/off light.
			lightify_node_set_lamptype(node, LIGHTIFY_UNKNOWNTYPE | n );
			dbg(ctx, "unknown type %x for lamp %s. PLEASE REPORT A BUG AGAINST liblightify. \n",n, lightify_node_get_name(node));
			break;
		}

		dbg(ctx, "xtra-data: %x -- %x %x %x %x\n", msg[ANSWER_0x13_UNKNOWN1],
				msg[ANSWER_0x13_UNKNOWN2],msg[ANSWER_0x13_UNKNOWN3],
				msg[ANSWER_0x13_UNKNOWN4],msg[ANSWER_0x13_UNKNOWN5]);


		lightify_node_set_red(node, msg[ANSWER_0x13_NODE_R]);
		lightify_node_set_green(node, msg[ANSWER_0x13_NODE_G]);
		lightify_node_set_blue(node, msg[ANSWER_0x13_NODE_B]);
		lightify_node_set_white(node, msg[ANSWER_0x13_NODE_W]);
		lightify_node_set_cct(node,	uint16_from_msg(&msg[ANSWER_0x13_NODE_CCT_LSB]));
		lightify_node_set_onoff(node, msg[ANSWER_0x13_NODE_ONOFF_STATE] != 0);
		lightify_node_set_online_status(node, msg[ANSWER_0x13_NODE_ONLINE_STATE]);
		lightify_node_set_brightness(node,msg[ANSWER_0x13_NODE_DIM_LEVEL]);
		lightify_node_set_stale(node, 0);
	}
	return 0;
}

LIGHTIFY_EXPORT int lightify_request_set_onoff(struct lightify_ctx *ctx, struct lightify_node *node, int onoff) {
	unsigned char msg[32];
	int n;
	if (!ctx) return -EINVAL;

	/* normalize to boolean -- int are 16bits...*/
	onoff = (onoff != 0);

	uint64_t node_adr = -1;
	uint64_t node_adr2;
	if (node) node_adr = lightify_node_get_nodeadr(node);

	long token = ++ctx->cnt;
	fill_telegram_header(msg, QUERY_0x32_SIZE, token, 0x00, 0x32);

	msg_from_uint64(&msg[QUERY_0x32_NODEADR64_B0], node_adr);
	msg[QUERY_0x32_ONOFF] = onoff;

	n = ctx->socket_write_fn(ctx,msg, QUERY_0x32_SIZE);
	if ( n < 0 ) {
		info(ctx,"socket_write_fn error %d\n", n);
		return n;
	}
	if ( n != QUERY_0x32_SIZE) {
		info(ctx,"short write %d!=%d\n", QUERY_0x32_SIZE, n);
		return -EIO;
	}

	/* read the header */
	n = ctx->socket_read_fn(ctx, msg, ANSWER_0x32_SIZE);
	if (n < 0) {
		info(ctx,"socket_read_fn error %d\n", n);
		return n;
	}
	if (n != ANSWER_0x32_SIZE) {
		info(ctx,"short read %d!=%d\n", ANSWER_0x32_SIZE, n);
		int i = 0;
		while(n--) {
		  info(ctx, " %d => %x\n ",i, msg[i++]);
		}
		info(ctx, "\n");
		return -EIO;
	}

	/* check the header if plausible */
	n = check_header_response(msg, token, 0x32);
	if ( n < 0 ) {
		info(ctx,"Invalid response (header)\n");
		return n;
	}

	/* check if the node adress was echoed properly */
	node_adr2 = uint64_from_msg(&msg[ANSWER_0x32_NODEADR64_B0]);
	if (node_adr != node_adr2) {
		info(ctx, "unexected node mac adr %llx!=%llx", node_adr, node_adr2 );
		return -EPROTO;
	}

	n = -decode_status(msg[ANSWER_0x32_STATE]);
	if (n) {
		info(ctx, "state %d indicates error.\n", n);
	}

	// Update node cache and if there was an error set stale status.
	if (!node) {
		while ((node=lightify_get_next_node(ctx,node))) {
			lightify_node_set_onoff(node,onoff);
			if (n) lightify_node_set_stale(node,1);
		}
	}
	else {
		lightify_node_set_onoff(node,onoff);
		if (n) lightify_node_set_stale(node,1);
	}
	return n;
}

LIGHTIFY_EXPORT int lightify_request_set_cct(struct lightify_ctx *ctx, struct lightify_node *node, unsigned int cct, unsigned int fadetime) {
	unsigned char msg[32];
	int n;
	if (!ctx) return -EINVAL;
	if (!node)return -EINVAL;

	uint64_t node_adr = lightify_node_get_nodeadr(node);

	long token = ++ctx->cnt;
	fill_telegram_header(msg, QUERY_0x33_SIZE, token, 0x00, 0x33);
	msg_from_uint64(&msg[QUERY_0x33_NODEADR64_B0], node_adr);
	msg[QUERY_0x33_CCT_LSB] = cct & 0xff;
	msg[QUERY_0x33_CCT_MSB] = (cct >> 8 ) & 0xff;
	msg[QUERY_0x33_FADETIME_LSB] = fadetime & 0xff;
	msg[QUERY_0x33_FADETIME_MSB] = (fadetime >> 8 ) & 0xff;

	n = ctx->socket_write_fn(ctx,msg, QUERY_0x33_SIZE);
	if ( n < 0 ) {
		info(ctx,"socket_write_fn error %d\n", n);
		return n;
	}
	if ( n != QUERY_0x33_SIZE) {
		info(ctx,"short write %d!=%d\n", QUERY_0x33_SIZE, n);
		return -EIO;
	}

	/* read the header */
	n = ctx->socket_read_fn(ctx, msg, ANSWER_0x33_SIZE);
	if (n < 0) {
		info(ctx,"socket_read_fn error %d\n", n);
		return n;
	}
	if (n != ANSWER_0x33_SIZE) {
		info(ctx,"short read %d!=%d\n", ANSWER_0x33_SIZE, n);
		return -EIO;
	}

	/* check the header if plausible */
	n = check_header_response(msg, token, 0x33);
	if ( n < 0 ) {
		info(ctx,"Invalid response (header)\n");
		return n;
	}

	/* check if the node address was echoed properly */
	if (node_adr != uint64_from_msg(&msg[ANSWER_0x33_NODEADR64_B0])) return -EPROTO;

	n = -decode_status(msg[ANSWER_0x33_STATE]);
	lightify_node_set_cct(node, cct);
	if (n) lightify_node_set_stale(node, 1);

	return n;
}

LIGHTIFY_EXPORT int lightify_request_set_rgbw(struct lightify_ctx *ctx,
		struct lightify_node *node, unsigned int r, unsigned int g,
		unsigned int b,unsigned int w,unsigned int fadetime) {
	unsigned char msg[32];
	int n;
	if (!ctx) return -EINVAL;
	/* does not support broadcast. */
	if (!node)return -EINVAL;

	uint64_t node_adr = lightify_node_get_nodeadr(node);

	long token = ++ctx->cnt;
	fill_telegram_header(msg, QUERY_0x36_SIZE, token, 0x00, 0x36);
	msg_from_uint64(&msg[QUERY_0x36_NODEADR64_B0], node_adr);
	msg[QUERY_0x36_R] = r & 0xff;
	msg[QUERY_0x36_G] = g & 0xff;
	msg[QUERY_0x36_B] = b & 0xff;
	msg[QUERY_0x36_W] = w & 0xff;
	msg[QUERY_0x36_FADETIME_LSB] = fadetime & 0xff;
	msg[QUERY_0x36_FADETIME_MSB] = (fadetime >> 8 ) & 0xff;

	n = ctx->socket_write_fn(ctx,msg, QUERY_0x36_SIZE);
	if ( n < 0 ) {
		info(ctx,"socket_write_fn error %d\n", n);
		return n;
	}
	if ( n != QUERY_0x36_SIZE) {
		info(ctx,"short write %d!=%d\n", QUERY_0x36_SIZE, n);
		return -EIO;
	}

	/* read the header */
	n = ctx->socket_read_fn(ctx, msg, ANSWER_0x36_SIZE);
	if (n != ANSWER_0x36_SIZE) {
		info(ctx,"short read %d!=%d\n", ANSWER_0x36_SIZE, n);
		return -EIO;
	}

	/* check the header if plausible */
	n = check_header_response(msg, token, 0x36);
	if ( n < 0 ) {
		info(ctx,"Invalid response (header)\n");
		return n;
	}

	/* check if the node adress was echoed properly */
	if (node_adr != uint64_from_msg(&msg[ANSWER_0x36_NODEADR64_B0])) return -EPROTO;

	n = -decode_status(msg[ANSWER_0x36_STATE]);

	lightify_node_set_red(node, r);
	lightify_node_set_green(node, g);
	lightify_node_set_blue(node, b);
	lightify_node_set_white(node, w);

	if (n) lightify_node_set_stale(node, 1);
	return n;

}

LIGHTIFY_EXPORT int lightify_request_set_brightness(struct lightify_ctx *ctx,
		struct lightify_node *node, unsigned int level, unsigned int fadetime) {
	unsigned char msg[32];
	int n;
	if (!ctx) return -EINVAL;
	if (!node)return -EINVAL;

	uint64_t node_adr = lightify_node_get_nodeadr(node);

	long token = ++ctx->cnt;
	fill_telegram_header(msg, QUERY_0x31_SIZE, token, 0x00, 0x31);
	msg_from_uint64(&msg[QUERY_0x31_NODEADR64_B0], node_adr);

	msg[QUERY_0x31_LEVEL] = level & 0xff;
	msg[QUERY_0x31_FADETIME_LSB] = fadetime & 0xff;
	msg[QUERY_0x31_FADETIME_MSB] = (fadetime >> 8 ) & 0xff;

	n = ctx->socket_write_fn(ctx,msg, QUERY_0x31_SIZE);
	if ( n < 0 ) {
		info(ctx,"socket_write_fn error %d\n", n);
		return n;
	}
	if ( n != QUERY_0x31_SIZE) {
		info(ctx,"short write %d!=%d\n", QUERY_0x31_SIZE, n);
		return -EIO;
	}

	/* read the header */
	n = ctx->socket_read_fn(ctx,msg,ANSWER_0x31_SIZE);
	if (n < 0) {
		info(ctx,"socket_read_fn error %d\n", n);
		return n;
	}
	if (n != ANSWER_0x31_SIZE) {
		info(ctx,"short read %d!=%d\n", ANSWER_0x31_SIZE, n);
		return -EIO;
	}

	n = check_header_response(msg, token, 0x31);
	if ( n < 0 ) {
		info(ctx,"Invalid response (header)\n");
		return n;
	}

	/* check if the node adress was echoed properly */
	if (node_adr != uint64_from_msg(&msg[ANSWER_0x31_NODEADR64_B0])) return -EPROTO;

	n = -decode_status(msg[ANSWER_0x31_STATE]);

	dbg(ctx, "unknown-bytes: %x %x %x\n", msg[ANSWER_0x31_UNKNOWN1],msg[ANSWER_0x31_UNKNOWN2],msg[ANSWER_0x31_UNKNOWN3]);

	lightify_node_set_brightness(node, level);
	lightify_node_set_onoff(node, level!=0); /* tweaking the level does that too */
	if (n) lightify_node_set_stale(node, 1);
	return n;
}

LIGHTIFY_EXPORT int lightify_request_update_node(struct lightify_ctx *ctx,
		struct lightify_node *node) {
	unsigned char msg[32];
	int n;
	if (!ctx) return -EINVAL;
	if (!node)return -EINVAL;

	uint64_t node_adr = lightify_node_get_nodeadr(node);
	long token = ++ctx->cnt;
	fill_telegram_header(msg, QUERY_0x68_SIZE, token, 0x00, 0x68);
	msg_from_uint64(&msg[QUERY_0x68_NODEADR64_B0], node_adr);

	n = ctx->socket_write_fn(ctx,msg, QUERY_0x68_SIZE);
	if ( n < 0 ) {
		info(ctx,"socket_write_fn error %d\n", n);
		return n;
	}
	if ( n != QUERY_0x68_SIZE) {
		info(ctx,"short write %d!=%d\n", QUERY_0x68_SIZE, n);
		return -EIO;
	}

	/* read the header */
	n = ctx->socket_read_fn(ctx,msg,ANSWER_0x68_SIZE);
	if (n < 0) {
		info(ctx,"socket_read_fn error %d\n", n);
		return n;
	}
	if (n != ANSWER_0x68_SIZE) {
		info(ctx,"short read %d!=%d\n", ANSWER_0x68_SIZE, n);
		return -EIO;
	}

	n = check_header_response(msg, token, 0x68);
	if ( n < 0 ) {
		info(ctx,"Invalid response (header)\n");
		return n;
	}

	/* check if the node adress was echoed properly */
	if (node_adr != uint64_from_msg(&msg[ANSWER_0x68_NODEADR64_B0])) return -EPROTO;

	/* update node information */
	lightify_node_set_online_status(node,msg[ANSWER_0x68_ONLINESTATE]);
	lightify_node_set_onoff(node,msg[ANSWER_0x68_ONOFF] != 0 );
	lightify_node_set_brightness(node,msg[ANSWER_0x68_DIM_LEVEL]);
	n = msg[ANSWER_0x68_CCT_LSB] | msg[ANSWER_0x68_CCT_MSB] << 8;
	lightify_node_set_cct(node,n);
	lightify_node_set_red(node,msg[ANSWER_0x68_R]);
	lightify_node_set_green(node,msg[ANSWER_0x68_G]);
	lightify_node_set_blue(node,msg[ANSWER_0x68_B]);
	lightify_node_set_white(node,msg[ANSWER_0x68_W]);

	n = -decode_status(msg[ANSWER_0x68_STATE]);
	lightify_node_set_stale(node, (n!=0));
	return n;
}
