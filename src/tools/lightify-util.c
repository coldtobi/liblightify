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

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

#include <liblightify/liblightify.h>
#include <unistd.h>

#include <fcntl.h>

#include <getopt.h>
#include <time.h>

#include <errno.h>


/* Flag set by ‘--verbose’. */
static int verbose_flag;

static struct option long_options[] = {
/* These options set a flag. */
{ "verbose", no_argument, &verbose_flag, 1 },
{ "brief", no_argument,   &verbose_flag, 0 },
/* These options don’t set a flag.
 We distinguish them by their indices. */
{ "cct", required_argument, 0, 'c' },
{ "rgbw", required_argument, 0, 'r' },
{ "level", required_argument, 0, 'l' },
{ "name", required_argument, 0,	'n' },
{ "host", required_argument, 0, 'h' },
{ "port", required_argument, 0, 'p' },
{ "on", required_argument, 0, '0' },
{ "off", required_argument, 0, '1' },
{ "time", required_argument, 0, 't' },
{ "list-nodes", no_argument, 0, 'd' },
{ "wait", required_argument, 0, 'w' },
{ "list-groups", no_argument, 0, 2},
{ "group", required_argument, 0, 'g' },
{ "update", no_argument, 0, 'u' },
{ "colorloop", no_argument, 0, 'z' },
{ "cctloop", no_argument, 0, 'y' },
{ 0, 0, 0, 0 }
};
/* getopt_long stores the option index here. */

// commands data
int command_cct = 0;
int command_cct_data = 0;

int command_r = 0;
int command_r_r = 0;
int command_r_g = 0;
int command_r_b = 0;
int command_r_w = 0;

int command_l = 0;
int command_l_data = 0;

//int name = 0;

char *name_data = NULL;

char *group_data = NULL;

char *host_data = NULL;

int port = 4000;

int fadetime;

int gonnected = 0;

int sockfd;

void usage(char *argv[]) {
	printf("Usage: %s [OPTIONS] \n", argv[0]);
	printf("     --host,-h <host>    Hostname or IP\n");
	printf("    [--verbose]          Verbose mode\n");
	printf("    [--list-nodes,-d]          Dump info about lamps\n");
	printf("    [--list-groups]       Show all known groups\n");
	printf("    [--wait,-w <value>]  Wait for value/10 seconds\n");
	printf("    [--time,-t <value>]  Set fading time  in 1/10 seconds\n");
	printf("    [--name,-n <value>]  Name of the lamp to be manipulated\n");
	printf("    [--group,-g <value>]  Name of the lamp to be manipulated\n");
	printf("    [--port,-p <port>]   Port, default 4000\n");
	printf("    [--on,-0]            Turn lamp on\n");
	printf("    [--off,-1]           Turn lamp off\n");
	printf("    [--level,-l <value>] Set intensity level. Range 0 to 100\n");
	printf("    [--cct,-c <value>]   CCT to be set.\n");
	printf("    [--rgbw,-r <value>]  Set color. Give color as r,g,b,w. Color values from 0 to 255\n");
	printf("    [--update,-u]        Refresh a node's information (requires name set before)\n");
	printf("\n Host must be given before any command. Commands on and off can broadcast to all lamps if name is not given before.\n");
	printf("\n All other commands needs either a name or group set before.\n");
}

struct lightify_node* find_node_per_name(struct lightify_ctx *ctx, const char *name) {
	if (!name) return NULL;
	struct lightify_node *node = NULL;
	while ((node = lightify_node_get_next(ctx, node))) {
		if (0 == strcmp(name, lightify_node_get_name(node))) {
			return node;
		}
	}
	fprintf(stderr, "ERROR: Node %s not found\n", name);
	return NULL;
}

struct lightify_group* find_grp_per_name(struct lightify_ctx *ctx, const char *name) {
	if (!name) return NULL;
	struct lightify_group *group = NULL;
	while( (group = lightify_group_get_next(ctx, group))) {
		if (0 == strcmp(name, lightify_group_get_name(group))) {
			return group;
		}
	}
	fprintf(stderr, "ERROR: Group %s not found\n", name);
	return NULL;
}

void command_set_0_1(struct lightify_ctx *ctx, int command_on) {
	struct lightify_node *node = find_node_per_name(ctx,name_data);;
	struct lightify_group *grp = find_grp_per_name(ctx,group_data);
	char *type, *name;

	command_on = command_on > 0 ? 1 : 0;
	if (!name_data && !group_data) {
		type = "Broadcast"; name = "";
		lightify_node_request_onoff(ctx, NULL, command_on);
	} else if (node) {
		type = "Node"; name = name_data;
		lightify_node_request_onoff(ctx, node, command_on);
	} else if (grp) {
		type = "Group"; name = group_data;
		lightify_group_request_onoff(ctx,grp,command_on);
	} else {
		return;
	}

	if (verbose_flag) {
		printf("%s %s switch %s\n", type, name , command_on ? "on" : "off");
	}
}

void command_set_cct(struct lightify_ctx *ctx) {
	struct lightify_node *node = find_node_per_name(ctx,name_data);;
	struct lightify_group *grp = find_grp_per_name(ctx,group_data);
	char *type, *name;

	if(node) {
		type = "Node"; name = name_data;
		lightify_node_request_cct(ctx, node, command_cct_data, fadetime);
	} else if (grp) {
		type = "Group"; name = group_data;
		lightify_group_request_cct(ctx, grp, command_cct_data, fadetime);
	} else {
		return;
	}

	if (verbose_flag) {
		printf("%s %s cct %dK in time %d\n", type, name, command_cct_data, fadetime );
	}
}

void command_set_rgbw(struct lightify_ctx *ctx) {
	struct lightify_node *node = find_node_per_name(ctx,name_data);
	struct lightify_group *grp = find_grp_per_name(ctx,group_data);
	char *type, *name;

	if(node) {
		type ="Node"; name = name_data;
		lightify_node_request_rgbw(ctx, node, command_r_r, command_r_g,
				command_r_b, command_r_w, fadetime);
	} else if (grp) {
		type ="Group"; name = group_data;
		lightify_group_request_rgbw(ctx, grp, command_r_r, command_r_g,
				command_r_b, command_r_w, fadetime);
	} else {
		return;
	}

	if (verbose_flag) {
		printf("%s %s rgbw %d,%d,%d,%d in time %d\n", type, name, command_r_r,
				command_r_g, command_r_b, command_r_w, fadetime);
	}
}

void command_set_lvl(struct lightify_ctx *ctx) {
	struct lightify_node *node = find_node_per_name(ctx,name_data);
	struct lightify_group *grp = find_grp_per_name(ctx,group_data);
	char *type, *name;

	if(node) {
		type ="Node"; name = name_data;
		lightify_node_request_brightness(ctx, node, command_l_data, fadetime);
	} else if (grp) {
		type ="Group"; name = group_data;
		lightify_group_request_brightness(ctx, grp, command_l_data, fadetime);
	} else {
		return;
	}

	if (verbose_flag) {
		printf("%s %s brightness %d in time %d\n", type, name, command_l_data, fadetime);
	}
}

void command_update_node(struct lightify_ctx *ctx) {
	struct lightify_node *node = find_node_per_name(ctx,name_data);
	if (!node) {
		return;
	}
	int ret = lightify_node_request_update(ctx,node);
	printf("update_node ret=%d\n", ret);
}

void command_do_colorloop(struct lightify_ctx *ctx) {
	struct lightify_node *node = find_node_per_name(ctx,name_data);
	if (!node) {
		return;
	}

	/* assemble some colorspec for testing.*/
	struct lightify_color_loop_spec colorspec[] = {
	 {  0x3c , 0x00 , 0xff , 0xff },
     {  0x05 , 15 , 0xff , 0xff },
     {  0x05 , 15 , 0xff , 0xff },
	 {  0x05 , 30 , 0xff , 0xff },
	 {  0x05 , 45 , 0xff , 0xff },
	 {  0x05 , 60 , 0xff , 0xff },
	 {  0x05 , 75 , 0xff , 0xff },
	 {	0x05 , 90 , 0xff , 0xff },
	 {  0x05 , 125 , 0xff , 0xff },
	 {	0x05 , 150 , 0xff , 0xff },
	 {	0x05 , 175 , 0xff , 0xff },
	 {	0x05 , 200 , 0xff , 0xff },
	 {	0x05 , 225 , 0xff , 0xff },
	 {	0x05 , 250 , 0xff , 0xff },
	 {	0x05 , 255 , 0xff , 0xff }
	};

	unsigned char statics[8] = {0x01, 0xff, 0x00, 0xff, 0x80, 0x3c, 0x00, 0x00};

	lightify_node_request_color_loop(ctx,  node, colorspec,
				sizeof(colorspec)/sizeof(struct lightify_color_loop_spec), NULL);
	return;
}

void command_do_cctloop(struct lightify_ctx *ctx) {
	struct lightify_node *node = find_node_per_name(ctx,name_data);
	if (!node) {
		return;
	}

	/* assemble some colorspec for testing.*/
	struct lightify_cct_loop_spec cctspec[] = {
		{  0x3c , 2000 , 0xff },
		{  0x05 , 2500 , 0xff },
		{  0x05 , 2700 , 0xff },
		{  0x05 , 3000 , 0xff },
		{  0x05 , 3500 , 0xff },
		{  0x05 , 4000 , 0xff },
		{  0x05 , 5000 , 0xff },
		{  0x05 , 6000 , 0xff },
		{  0x05 , 5000 , 0xff },
		{  0x05 , 4000 , 0xff },
		{  0x05 , 3500 , 0xff },
		{  0x05 , 3000 , 0xff },
		{  0x05 , 2700 , 0xff },
		{  0x05 , 2500 , 0xff },
		{  0x05 , 2000 , 0xff }
	};

	unsigned char statics[8] =
			{ 0x01, 0xff, 0x00, 0xff, 0x80, 0x3c, 0x00, 0x00 };
	lightify_node_request_cct_loop(ctx, node, cctspec,
			sizeof(cctspec) / sizeof(struct lightify_cct_loop_spec), NULL);

	return;
}



void setup_connection(struct lightify_ctx *ctx) {
	/* Create a socket point */
	int err;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) {
		perror("ERROR opening socket");
		exit(1);
	}
	server = gethostbyname(host_data);

	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		exit(1);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *) server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(port);

	/* Now connect to the server */
	if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("ERROR connecting");
		exit(1);
	}

	err = lightify_skt_setfd(ctx, sockfd);
	if (err < 0) {
		fprintf(stderr, "Could not set fd\n");
		exit(1);
	}

	gonnected = 1;

	// scan nodes
	err = lightify_node_request_scan(ctx);
	if (err < 0) {
		fprintf(stderr,
				"Error during node scan -- lets see if we've got partial data\n");
	}

	// scan groups
	err = lightify_group_request_scan(ctx);
	if (err < 0) {
		fprintf(stderr,
				"Error during group scan -- lets see if we've got partial group data\n");
	}
}


const char *decode_online_state(int state) {
	switch (state) {
		case  LIGHTIFY_OFFLINE:
			return "offline";
		case LIGHTIFY_ONLINE:
			return "online";
		default:
			return "unknown";
	}
}

const char *decode_lamptype(int type) {
	switch (type) {
		case LIGHTIFY_ONOFF_PLUG:
			return "oo-plug";
		case LIGHTIFY_DIMABLE_LIGHT:
			return "dim";
		case LIGHTIFY_COLOUR_LIGHT:
			return "color";
		case LIGHTIFY_EXT_COLOUR_LIGHT:
			return "ext-col";
		case LIGHTIFY_CCT_LIGHT:
			return "cct";
		case LIGHTIFY_4WAY_SWITCH:
			return "4way-sw";
		default:
			return "unknown";
	}
}

const char *decode_onoff_sate(int state) {
	if (state < 0) return "err";
	switch (state) {
		case  0:
			return "off";
		case 1:
			 return "on";
		default:
			return "???";
	}
}

void dump_nodes_state(struct lightify_ctx *ctx) {
	int count=0;
	// Let's create a short table...
	//      1234567890123456  1234567812345678
	printf("|------------------|------------------|---------|--------|---------|-----|-----|------|-----|-----|-----|-----|---|------|\n");
	printf("| Name             | MAC              | type    | group  | online  | 0/1 | dim | CCT  | Red | Grn | Blu | Wht | s | ZAdr |\n");
	printf("|------------------|------------------|---------|--------|---------|-----|-----|------|-----|-----|-----|-----|---|------|\n");
	struct lightify_node *node = NULL;

	while (( node  = lightify_node_get_next(ctx, node))) {
		count++;
		printf("| %-16s |" , lightify_node_get_name(node));
		printf(" %016lx |" , lightify_node_get_nodeadr(node));
		printf(" %-7s |", decode_lamptype(lightify_node_get_lamptype(node)));
		printf(" 0x%04x |", lightify_node_get_grpadr(node));
		printf(" %-7s |", decode_online_state(lightify_node_get_onlinestate(node)));
		printf(" %-3s |", decode_onoff_sate(lightify_node_is_on(node)));
		printf(" %-3d |", lightify_node_get_brightness(node));
		printf(" %-4d |", lightify_node_get_cct(node));
		printf(" %-3d |", lightify_node_get_red(node));
		printf(" %-3d |", lightify_node_get_green(node));
		printf(" %-3d |", lightify_node_get_blue(node));
		printf(" %-3d |", lightify_node_get_white(node));
		printf(" %c |", lightify_node_is_stale(node) ? '*' :' ');
		printf(" %-4x |\n", lightify_node_get_zoneadr(node));
	}

	if (!count) {
		printf("no nodes found\n");
	}
	printf("|------------------|------------------|---------|--------|---------|-----|-----|------|-----|-----|-----|-----|---|------|\n");
}

void dump_groups(struct lightify_ctx *ctx) {

	struct lightify_group *group = NULL;
	printf("|------------------|----|--------|----------------\n");
	printf("| Group Name       | id | mask   | Group members\n");
	printf("|------------------|----|--------|----------------\n");
	while ((group = lightify_group_get_next(ctx,group))) {
		printf("| %-16s | %-2d | 0x%04x |", lightify_group_get_name(group),
				lightify_group_get_id(group),
				1 << (lightify_group_get_id(group)-1));
		struct lightify_node *node = NULL;
		while ((node = lightify_group_get_next_node(group, node))) {
			printf(" %s", lightify_node_get_name(node));
		}
		printf("\n");
	}
	printf("|------------------|----|--------|----------------\n");
}



int main(int argc, char *argv[]) {
	int option_index = 0;

	int c, err;
	int n;

	struct lightify_ctx *ctx;

	err = lightify_new(&ctx, NULL );
	if (err < 0) {
		fprintf(stderr, "Cannot allocate library context\n");
		exit(1);
	}


	while (1) {
		c = getopt_long(argc, argv, "dc:r:l:n:h:p:01t:w:g:u:z:y", long_options,
				&option_index);
		if (c == -1)
			break;

		switch (c) {

		case '0':
			if (!host_data) { usage(argv); exit(1); }
			if (!gonnected) setup_connection(ctx);
			command_set_0_1(ctx,0);
			break;

		case '1':
			if (!host_data) { usage(argv); exit(1); }
			if (!gonnected) setup_connection(ctx);
			command_set_0_1(ctx,1);
			break;

		case 'c':
			if (!host_data || (!name_data && !group_data) ) {
				usage(argv);
				exit(1);
			}
			if (!gonnected) setup_connection(ctx);
			command_cct = 1;
			command_cct_data = strtol(optarg, NULL, 10);
			command_set_cct(ctx);
			break;

		case 'r':
			if (!host_data || (!name_data && !group_data)) {
				usage(argv);
				exit(1);
			}
			if (!gonnected) setup_connection(ctx);
			command_r = 1;
			sscanf(optarg, "%d,%d,%d,%d", &command_r_r, &command_r_g,
					&command_r_b, &command_r_w);
			command_set_rgbw(ctx);
			break;

		case 'l':
			if (!host_data || (!name_data && !group_data)) {
				usage(argv);
				exit(1);
			}
			if (!gonnected) setup_connection(ctx);
			command_l = 1;
			command_l_data = strtol(optarg, NULL, 10);
			command_set_lvl(ctx);
			break;

		case 'n':
			name_data = optarg;
			group_data = NULL;
			break;

		case 'h':
			host_data = optarg;
			break;

		case 'p':
			port = strtol(optarg, NULL, 10);
			break;

		case 't':
			fadetime = strtol(optarg, NULL, 10);
			break;

		case 'd': {
			if (!host_data) {
				usage(argv);
				exit(1);
			}
			if (!gonnected) setup_connection(ctx);
			dump_nodes_state(ctx);
			break;
		}

		case 'w': {
			int tme = strtol(optarg, NULL, 10);
			struct timespec ts;
			ts.tv_sec = tme / 10;
			ts.tv_nsec = (tme % 10 ) * 100 * 1000 * 1000;
			while (1) {
				if (0 == nanosleep(&ts,&ts)) break;
				if (errno != EINTR) break;
			}
		}
		break;

		case 2: {
			if (!gonnected) setup_connection(ctx);
			dump_groups(ctx);
			break;
		}

		case 'g': {
			if (!gonnected) setup_connection(ctx);
			group_data = optarg;
			name_data = NULL;
			break;
		}

		case 'u': {
			if (!gonnected || !name_data) { usage(argv); exit(1); }
			command_update_node(ctx);
			break;
		}

		case 'z': {
			if (!gonnected || !name_data) { usage(argv); exit(1); }
			command_do_colorloop(ctx);
			break;
		}

		case 'y': {
			if (!gonnected || !name_data) { usage(argv); exit(1); }
			command_do_cctloop(ctx);
			break;
		}

		case 0:
			break;
		case 1:
			break;

		default:
			//printf("unknown option %c %d", c, c);
			usage(argv);
			exit(1);
		}
	}

	return 0;
}
