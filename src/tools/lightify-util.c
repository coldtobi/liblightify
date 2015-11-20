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
{ "dump", no_argument, 0, 'd' },
{ "wait", required_argument, 0, 'w' },
{ "groups", no_argument, 0, 'g' },
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

int command_on = 0;

int command_l = 0;
int command_l_data = 0;

int name = 0;

char *name_data = NULL;

char *host_data = NULL;

int port = 4000;

int fadetime;

int gonnected = 0;

int sockfd;

void usage(char *argv[]) {
	printf("Usage: %s [OPTIONS] \n", argv[0]);
	printf("     --host,-h <host>    Hostname or IP\n");
	printf("    [--verbose]          Verbose mode\n");
	printf("    [--name,-n <value>]  Name of the lamp to be manipulated\n");
	printf("    [--port,-p <port>]   Port, default 4000\n");
	printf("    [--dump,-d]          Dump info about lamps\n");
	printf("    [--on,-0]            Turn lamp on\n");
	printf("    [--off,-1]           Turn lamp off\n");
	printf("    [--level,-l <value>] Set intensity level. Range 0 to 100\n");
	printf("    [--cct,-c <value>]   CCT to be set.\n");
	printf("    [--rgbw,-r <value>]  Set color. Give color as r,g,b,w. Color values from 0 to255\n");
	printf("    [--wait,-w <value>]  Wait for value/10 seconds\n");
	printf("\n Host must be given before any command. Commands on and off can broadcast to all lamps if name is not given before.\n");
}

void command_set_0_1(struct lightify_ctx *ctx) {
	struct lightify_node *node;
	if (command_on) {
		command_on = command_on > 0 ? 1 : 0;
		if (!name_data) {
			lightify_request_set_onoff(ctx, 0, command_on);
		} else {
			node = NULL;
			while ((node = lightify_get_next_node(ctx, node))) {
				if (0 == strcmp(name_data, lightify_node_get_name(node))) {
					if (verbose_flag) printf("node %s %s\n", lightify_node_get_name(node), command_on ? "on" : "off");
					lightify_request_set_onoff(ctx, node, command_on);
					break;
				}
			}
			if (!node) {
				printf("node %s not found\n", name_data);
			}
		}
	}
}

void command_set_cct(struct lightify_ctx *ctx) {
	struct lightify_node *node;
	node = NULL;
	while ((node = lightify_get_next_node(ctx, node))) {
		if (0 == strcmp(name_data, lightify_node_get_name(node))) {
			if (verbose_flag) printf("node %s cct %d in time %d\n",
					lightify_node_get_name(node), command_cct_data,
					fadetime);
			lightify_request_set_cct(ctx, node, command_cct_data, fadetime);
			break;
		}
	}
	if (!node) {
		printf("node %s not found\n", name_data);
	}
}

void command_set_rgbw(struct lightify_ctx *ctx) {
	struct lightify_node *node;
	node = NULL;
	while ((node = lightify_get_next_node(ctx, node))) {
		if (0 == strcmp(name_data, lightify_node_get_name(node))) {
			if (verbose_flag) printf("node %s rgbw %d,%d,%d,%d in time %d\n",
					lightify_node_get_name(node), command_r_r,
					command_r_g, command_r_b, command_r_w, fadetime);
			lightify_request_set_rgbw(ctx, node, command_r_r,
					command_r_g, command_r_b, command_r_w, fadetime);
			break;
		}
	}
	if (!node) {
		printf("node %s not found\n", name_data);
	}
}

void command_set_lvl(struct lightify_ctx *ctx) {
	struct lightify_node *node;
	node = NULL;
	while ((node = lightify_get_next_node(ctx, node))) {
		if (0 == strcmp(name_data, lightify_node_get_name(node))) {
			if (verbose_flag) printf("node %s rgbw brightness %d in time %d\n",
					lightify_node_get_name(node), command_l_data , fadetime);
			lightify_request_set_brightness(ctx, node, command_l_data, fadetime);
			break;
		}
	}
	if (!node) {
		printf("node %s not found\n", name_data);
	}
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
	err = lightify_scan_nodes(ctx);
	if (err < 0) {
		fprintf(stderr,
				"Error during scan -- lets see if we've got partial data\n");
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
	printf("|-----------------|------------------|---------|-------|---------|-----|-----|------|-----|-----|-----|-----|---|\n");
	printf("| Name            | MAC              | type    | group | online  | 0/1 | dim | CCT  | Red | Grn | Blu | Wht | s |\n");
	printf("|-----------------|------------------|---------|-------|---------|-----|-----|------|-----|-----|-----|-----|---|\n");
	struct lightify_node *node = NULL;

	while (( node  = lightify_get_next_node(ctx, node))) {
		count++;
		printf("|%-16s |" , lightify_node_get_name(node));
		printf(" %016llx |" , lightify_node_get_nodeadr(node));
		printf(" %-7s |", decode_lamptype(lightify_node_get_lamptype(node)));
		printf(" %-5d |", lightify_node_get_grpadr(node));
		printf(" %-7s |", decode_online_state(lightify_node_get_onlinestate(node)));
		printf(" %-3s |", decode_onoff_sate(lightify_node_is_on(node)));
		printf(" %-3d |", lightify_node_get_brightness(node));
		printf(" %-4d |", lightify_node_get_cct(node));
		printf(" %-3d |", lightify_node_get_red(node));
		printf(" %-3d |", lightify_node_get_green(node));
		printf(" %-3d |", lightify_node_get_blue(node));
		printf(" %-3d |", lightify_node_get_white(node));
		printf(" %c |\n", lightify_node_is_stale(node) ? '*' :' ');
	}

	if (!count) {
		printf("no nodes found\n");
	}
	printf("|-----------------|------------------|---------|-------|---------|-----|-----|------|-----|-----|-----|-----|---|\n");
}

void dump_groups(struct lightify_ctx *ctx) {

	int i = lightify_request_scan_groups(ctx);
	printf("Scan groups: Result %d\n", i);

	if (i > 0) {
		struct lightify_group *group = NULL;
		printf("| Group Name       | id     |\n");
		printf("|------------------|--------|\n");
		while ((group = lightify_group_get_next_group(ctx,group))) {
			printf("| %-16s | 0x%04x |\n", lightify_group_get_name(group), lightify_group_get_id(group));
		}
		printf("|------------------|--------|\n");
	}
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
		c = getopt_long(argc, argv, "dc:r:l:n:h:p:01t:w:", long_options,
				&option_index);
		if (c == -1)
			break;

		switch (c) {

		case '0':
			if (!host_data) {
				exit(1);
			}
			if (!gonnected) setup_connection(ctx);
			command_on = -1;
			command_set_0_1(ctx);
			break;

		case '1':
			if (!host_data) {
				usage(argv);
				exit(1);
			}
			if (!gonnected) setup_connection(ctx);
			command_on = 1;
			command_set_0_1(ctx);
			break;

		case 'c':
			if (!host_data || !name) {
				usage(argv);
				exit(1);
			}
			if (!gonnected) setup_connection(ctx);
			command_cct = 1;
			command_cct_data = strtol(optarg, NULL, 10);
			command_set_cct(ctx);
			break;

		case 'r':
			if (!host_data || !name) {
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
			if (!host_data || !name) {
				usage(argv);
				exit(1);
			}
			if (!gonnected) setup_connection(ctx);
			command_l = 1;
			command_l_data = strtol(optarg, NULL, 10);
			command_set_lvl(ctx);
			break;

		case 'n':
			name = 1;
			name_data = optarg;
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

		case 'g': {
			if (!gonnected) setup_connection(ctx);
			dump_groups(ctx);
			break;
		}

		default:
			usage(argv);
			exit(1);
		}
	}

	return 0;
}
