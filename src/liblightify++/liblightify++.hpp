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

/**
 * \file lightify++.h
 *
 *  Created on: 22.09.2015
 *      Author: Tobias Frost
 */

#ifndef SRC_LIBLIGHTIFY___LIGHTIFY___HPP_
#define SRC_LIBLIGHTIFY___LIGHTIFY___HPP_


#include <liblightify/liblightify.h>
#include <string.h>

#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

/// Enable the use of exception within this wrapper
#undef LIGHTIFY_ALLOW_THROW
#ifdef LIGHTIFY_ALLOW_THROW
#include <stdexcept>
#endif

class Lightify_Node {
	friend class Lightify;
protected:
	Lightify_Node(struct lightify_ctx *ctx,
			struct lightify_node *node) {
		_ctx = ctx;
		_node = node;
	}

public:
	// NOTE: Getters of node information will generally access cached data.
	// An update from the actual node data has to be explicitly requested.
	// An exception is the Name: It can only be queried with a scan.

	/** Get the node's name. */
	const char *GetName(void) const {
		return lightify_node_get_name(_node);
	}

	/** Get the node's MAC */
	unsigned long long GetMAC(void) const {
		return lightify_node_get_nodeadr(_node);
	}

	/** Get the node's zone address */
	unsigned int GetZoneAdr(void) const{
		return lightify_node_get_zoneadr(_node);
	}

	/** Get the node's group  */
	unsigned int GetGroup(void) const{
		return lightify_node_get_grpadr(_node);
	}

	/** Get the lamp type  */
	lightify_node_type GetLampType(void)const {
		return lightify_node_get_lamptype(_node);
	}

	/** Get the current color value: red */
	int GetRed(void) const {
		return lightify_node_get_red(_node);
	}

	/** Get the current color value: blue */
	int GetBlue(void) const {
		return lightify_node_get_blue(_node);
	}

	/** Get the current color value: green */
	int GetGreen(void) const {
		return lightify_node_get_green(_node);
	}

	/** Get the current color value: white */
	int GetWhite(void) const {
		return lightify_node_get_white(_node);
	}

	/** Get the CCT */
	int GetCCT(void) const {
		return lightify_node_get_cct(_node);
	}

	/** Get the brightness */
	int GetBrightness(void) const {
		return lightify_node_get_brightness(_node);
	}

	/** Is the lamp on? */
	int IsOn(void) const {
		return lightify_node_is_on(_node);
	}

	/** Get the online status? */
	int GetOnlineState(void) const {
		return lightify_node_get_onlinestate(_node);
	}

	/** Is the lamp info stale?
	 *  (that is: a previous request failed, we do not know the status.)
	*/
	int IsStale(void) const {
		return lightify_node_is_stale(_node);
	}

	/* The setter functions actually talk with the nodes.
	 * After setting the values are *not* verified from the hardware,
	 * only the cache will be updated.
	 *
	 * */

	/** Turn on / off */
	int TurnOnOff(bool onoff) {
		return lightify_node_request_onoff(_ctx, _node, onoff);
	}

	/** Set color temperature
	 *
	 *  \param cct to be set
	 *  \param time fadetime for transition in 1/10 secs
	 *
	 *  \returns negative on error
	 */
	int SetCCT(int cct, int time) {
		return lightify_node_request_cct(_ctx, _node, cct, time);
	}

	/** Set color components
	 *
	 *  \note observed, that white can only be set when the other colors are 0.
	 *
	 *  Color values are good from 0-255
	 *
	 *  \param red to be set
	 *  \param green to be set
	 *  \param blue to be set
	 *  \param white to be set
	 *  \param time fadetime for transition in 1/10 secs
	 *
	 * \returns negative on error
	 * */
	int SetRGBW(int red, int green, int blue, int white, int time) {
		return lightify_node_request_rgbw(_ctx,_node, red, green, blue, white, time);
	}

	/** Set brightness
	 *
	 *  \param level 0..100
	 *  \param time time in 1/10s
	 *
	 *  \returns negative on error
	 * */
	int SetBrightness(int level, int time) {
		return lightify_node_request_brightness(_ctx, _node, level, time);
	}

	/** Query updated node information from the gateway
	 *
	 * Will ask the gateway to refresh node's information.
	 *
	 * @return negative on error, 0 on success
	 *
	 * \sa lightify_request_update_node()
	 */
	int UpdateNodeInfo(void) {
		return lightify_node_request_update(_ctx, _node);
	}


private:
	Lightify_Node(const Lightify_Node &other) {
		// No copies!
	}

private:
	struct lightify_node *_node;
	struct lightify_ctx *_ctx;
};

class Lightify_Group {
	friend class Lightify;
protected:
	Lightify_Group(struct lightify_ctx *ctx,
			struct lightify_group *group) {
		_ctx = ctx;
		_group = group;
	}

public:
	int GetId() {
		return lightify_group_get_id(_group);
	}

	const char *GetName() {
		return lightify_group_get_name(_group);
	}

	/** Turn on / off */
	int TurnOnOff(bool onoff) {
		return lightify_group_request_onoff(_ctx, _group, onoff);
	}

	/** Set color temperature
	 *
	 *  \param cct to be set
	 *  \param time fadetime for transition in 1/10 secs
	 *
	 *  \returns negative on error
	 */
	int SetCCT(int cct, int time) {
		return lightify_group_request_cct(_ctx, _group, cct, time);
	}

	/** Set color components
	 *
	 *  \note observed, that white can only be set when the other colors are 0.
	 *
	 *  Color values are good from 0-255
	 *
	 *  \param red to be set
	 *  \param green to be set
	 *  \param blue to be set
	 *  \param white to be set
	 *  \param time fadetime for transition in 1/10 secs
	 *
	 * \returns negative on error
	 * */
	int SetRGBW(int red, int green, int blue, int white, int time) {
		return lightify_group_request_rgbw(_ctx,_group, red, green, blue, white, time);
	}

	/** Set brightness
	 *
	 *  \param level 0..100
	 *  \param time time in 1/10s
	 *
	 *  \returns negative on error
	 * */
	int SetBrightness(int level, int time) {
		return lightify_group_request_brightness(_ctx, _group, level, time);
	}


private:
	Lightify_Group(const Lightify_Group &) {
		// No copies!
		// Just to avoid warnings..
		_ctx = NULL; _group = NULL;
	}



private:
	struct lightify_group *_group;
	struct lightify_ctx *_ctx;
};

/** Lightify-Class to encapsulate the library context and offer
 * access to the management functionality.
 *
 * \note throws std::bad_alloc on out of memory, when exceptions enabled.
 * (see LIGHTIFY_ALLOW_THROW)
 * */
class Lightify {
public:
	/// Default Constructor
	/// Throws invalid_argument if port is >0xffff.
	Lightify(const char *host, unsigned int port=4000) {
		_host = 0;
		if (host) {
			_host = strdup(host);
		}
		_port = port;
		_sockfd = -1;
		_ctx = NULL;
		_nodesmap = NULL;
		_groupsmap = NULL;
		_no_nodes = 0;
		_no_groups = 0;

		int err = lightify_new(&_ctx, NULL);
#ifdef LIGHTIFY_ALLOW_THROW
		if (err < 0 || !_ctx) {
			throw std::bad_alloc;
		}
#endif
	}

	virtual ~Lightify() {
		if (_ctx) lightify_free(_ctx);
		if (_host) free(_host);
		if (_sockfd != -1) close(_sockfd);
		_free_nodemap();
		_free_groupmap();
	}

	/** Open socket / prepare communication
	  * returns 0 on success, negative on error.*/
	int Open(void) {

		if (_sockfd > 0) {
			int e = Close();
			if (e < 0) return e;
		}

		int err;
		struct sockaddr_in serv_addr;
		struct hostent *server;

		_sockfd = socket(AF_INET, SOCK_STREAM, 0);

		if (_sockfd < 0) {
			err = _sockfd;
			_sockfd = -1;
			return err;
		}

		// FIXME gethostbyname is depreciated.
		server = gethostbyname(_host);
		if (server == NULL) {
			// no such host
			err = h_errno;
			if (err >= 0) err = -EHOSTUNREACH;
			goto err_out;
		}

		memset((char *) &serv_addr, 0, sizeof(serv_addr));
		serv_addr.sin_family = AF_INET;
		memcpy((char *) &serv_addr.sin_addr.s_addr, (char *) server->h_addr,
				server->h_length);
		serv_addr.sin_port = htons(_port);

		/* Now connect to the server */
		err = connect(_sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
		if (err < 0) {
			err = -errno;
			goto err_out;
		}

		err = lightify_skt_setfd(_ctx, _sockfd);
		if (err < 0) {
			goto err_out;
		}

		return 0;

err_out:
		if (_sockfd > 0) Close();
		return err;

	}

	/** Close the (previously) opened socket. */
	int Close(void) {
		int i = 20;
		if (_sockfd == -1) return -EBADF;
		lightify_skt_setfd(_ctx, -1);
		int local_socketfd = _sockfd;
		_sockfd = -1;
		/*  Retry interrupted system calls, but with capped iteration count.*/
		while (--i && 0 != close(local_socketfd)) {
			if (errno != EINTR) return errno;
		}
		if (!i) return -EINTR;
		return 0;
	}

	/** Scan for nodes
	 \note: This invalidates all previously known node information! */
	int ScanNodes(void) {

		int err;
		if (_sockfd == -1) return -EBADF;
		_free_nodemap();
		err = lightify_node_request_scan(_ctx);
		if (err < 0) return err;

		struct lean_nodemap *last_inserted = NULL;
		struct lightify_node *node = NULL;
		while(node = lightify_node_get_next(_ctx,node)) {
			struct lean_nodemap *nm = new lean_nodemap();
			if (!nm) return -ENOMEM;
			nm->next = 0;
			nm->node = new Lightify_Node(_ctx,node);
			if (!nm->node) {
				delete nm;
				return -ENOMEM;
			}

			if (!last_inserted) {
				_nodesmap = nm;
			} else {
				last_inserted->next = nm;
			}
			last_inserted = nm;
			_no_nodes ++;
		}
		return _no_nodes;

	}

	/** Scan for known groups and generate a Group object for every returned group.
	 *
	 * \note Scanning will invalidate all previous objects.
	 *
	 * \return number of detected groups, negative on error.
	 * */
	int ScanGroups(void) {
		int err;
		if (_sockfd == -1) return -EBADF;
		_free_groupmap();
		err = lightify_group_request_scan(_ctx);
		if (err < 0) return err;

		struct lean_groupmap *last_inserted = NULL;
		struct lightify_group *group = NULL;
		while ( (group = lightify_group_get_next(_ctx, group))) {
			struct lean_groupmap *gm = new lean_groupmap();
			if (!gm) return -ENOMEM;
			gm->next = NULL;
			gm-> group = new Lightify_Group(_ctx, group);
			if (!gm->group) {
				delete gm;
				return -ENOMEM;
			}

			if (!last_inserted) {
				_groupsmap = gm;
			} else {
				last_inserted->next = gm;
			}
			last_inserted = gm;
			_no_groups++;
		}
		return _no_groups;
	}

	/** Get direct access to the lighitfy context
	 *
	 * \warning advanced use -- improper use can confuse the C++ Wrapper
	 * and create inconsitent state.
	*/
	struct lightify_ctx *GetCtx(void) {
		return _ctx;
	}

	/** Actions that can be broadcasted. */
	int TurnAllOnOff(bool onoff)
	{
		return lightify_node_request_onoff(_ctx,NULL,onoff);
	}

	/** Get the node object for a given MAC address */
	Lightify_Node *GetNode(long long mac) {
		struct lean_nodemap *nm = _nodesmap;
		while(nm) {
			if (nm->node->GetMAC() == mac) return nm->node;
			nm = nm->next;
		}
		return NULL;
	}

	/** Get node at Pos X
	 *
	 * \return pointer or NULL
	 *
	 */
	Lightify_Node* GetNodeAtPosX(int x) const {
		if (x >= _no_nodes) return NULL;
		struct lean_nodemap *nm = _nodesmap;
		while(nm && x--) nm = nm->next;
		return (nm ? nm->node : NULL);
	}

	/** Get Group at Pos X
	 *
	 * \return pointer or NULL
	 *
	 */
	Lightify_Group* GetGroupAtPosX(int pos) const {
		if (pos >= _no_groups) return NULL;
		struct lean_groupmap *gm = _groupsmap;
		while(pos--) gm = gm->next;
		return gm->group;
	}

	/** Get the library context -- for direct library access.
	 * \warning dangerous! Avoid calls that might change e.g memory location of the node structures, like scan for nodes etc. */
	const struct lightify_ctx *GetLightifyContext(void) const {
		return _ctx;
	}

	/** Have the connection been opened?
	 *
	 * \note this only checks for an valid file descriptor
	 *
	 * \return true, yes*/
	int IsOpen(void) const
	{
		return (_sockfd > 0);
	}

	/** Set a hostname / port combination
	 *
	 * \param host string used as host (hostname of IP)
	 * \param port to be used, default 4000
	 *
	 * \return negative on error, otherwise on success.
	 * (Error causes are invalid parameters like host=NULL or port>0xffff
	 * and out of memory situations.)
	 */
	int SetHostname(const char *host, unsigned int port=4000) {
		if (!host) return -EINVAL;
		if (port > 0xFFFFUL) return -EINVAL;

		if (_host) free(_host);
		_host = strdup(host);
		if (!_host) return -ENOMEM;
		return 0;
	}


	int GetNodesCount(void) {
		return _no_nodes;
	}

	int GetGroupsCount(void) {
		return _no_groups;
	}


private:
	void _free_nodemap(void) {
		struct lean_nodemap *nmtmp, *nm = _nodesmap;
		while (nm) {
			nmtmp = nm->next;
			delete nm->node;
			delete nm;
			nm = nmtmp;
		}
		_nodesmap = NULL;
		_no_nodes = 0;
	}

	void _free_groupmap(void) {
		struct lean_groupmap *gmtmp, *gm = _groupsmap;
		while (gm) {
			gmtmp = gm->next;
			delete gm->group;
			delete gm;
			gm = gmtmp;
		}
		_groupsmap = NULL;
		_no_groups = 0;
	}

	struct lightify_ctx *_ctx;
	char *_host;
	unsigned int _port;
	int _sockfd;

	// this is to avoid avoid STL...
	struct lean_nodemap {
		struct lean_nodemap *next;
		Lightify_Node *node;
	};

	struct lean_groupmap {
		struct lean_groupmap *next;
		Lightify_Group *group;
	};

	struct lean_nodemap *_nodesmap;
	struct lean_groupmap *_groupsmap;

	int _no_nodes;
	int _no_groups;
};


#endif /* SRC_LIBLIGHTIFY___LIGHTIFY___HPP_ */
