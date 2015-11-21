/*
    lightify test suite

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

#include <check.h>
#include <errno.h>
#include <stdio.h>

#include <liblightify/liblightify.h>

#include <assert.h>

struct lightify_ctx *_ctx;

struct fake_socket {
	char *buf_read;
	char *buf_write;
	int size_read;
	int size_write;
	int err_read;  // << for error injection
	int err_write; // << for error injection
} my_fakesocket;


void print_protocol_mismatch_write(struct fake_socket *mfs, unsigned char *should) {
	char buf[512] ="\nWROTE:\n\n";

	int found = 0;
	int i = 0;
	for (i = 0; i < mfs->size_write; i++) {
		unsigned int one, two;
		one = 0xff & mfs->buf_write[i];
		two = 0xff & should[i];
		sprintf(buf + strlen(buf), " %02x%c=%02x", one,
				(one == two ? '=' : '!'), two);
		if (one != two) found =1;
	}
	if (found) ck_abort_msg(buf);
}

void print_protocol_mismatch_read(struct fake_socket *mfs, unsigned char *should) {
	char buf[512] ="\nREAD\n\n";

	int i = 0;
	int found = 0;
	for (i = 0; i < mfs->size_read; i++) {
		unsigned int one, two;
		one = 0xff & mfs->buf_read[i];
		two = 0xff & should[i];
		sprintf(buf + strlen(buf), " %02x%c=%02x", one,
				(one == two ? '=' : '!'), two);
		if (one != two) found =1;
	}
	if (found) ck_abort_msg(buf);
}


// testdata

// expected string to be sent to the gateway when scanning for nodes
const static char scanfornodes_query[] = {0x07,0x00,0x00,0x13,0x01,0x00,0x00,0x00,0x01 };

// dummy response for the scan for nodes command
// (wiresharked, but edited to avoid same data e.g for the colors  ;-)
// MAC64 set to 0xdeadbeef12345678
const static char scanfornodes_answer[] = {
	0x33, 0x00, 0x01, 0x13, 0x01, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x00, 0x55, 0xaa,
	0x78, 0x56, 0x34, 0x12, 0xef, 0xbe, 0xad, 0xde,
        0x02, 0x01, 0x02, 0x03, 0x07,
	0x02, 0xcd, 0xab, 0x00, 0x64, 0x8e,
	0x0a, 0xf0, 0xf1, 0xf2, 0xf3, 0x4c, 0x69, 0x63,
	0x68, 0x74, 0x20, 0x30, 0x31, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00
};


const static char turnonlight_query_broadcast[] = {
	0x0f, 0x00, 0x00, 0x32, 0x02, 0x00, 0x00, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x01
};

const static char turnonlight_answer_broadcast[] = {
	0x12, 0x00, 0x01, 0x32, 0x02, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x00,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0x00
};

const static char turnonlight_query_node[] = {
	0x0f, 0x00, 0x00, 0x32, 0x03, 0x00, 0x00, 0x00,
	0x78, 0x56, 0x34, 0x12, 0xef, 0xbe, 0xad, 0xde,
	0x01
};

const static char turnonlight_answer_node[] = {
	0x12, 0x00, 0x01, 0x32, 0x03, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x00,
	0x78, 0x56, 0x34, 0x12, 0xef, 0xbe, 0xad, 0xde,
	0x00
};


const static char changecct_query_node[] = {
	0x12, 0x00, 0x00, 0x33, 0x04, 0x00, 0x00, 0x00,
	0x78, 0x56, 0x34, 0x12, 0xef, 0xbe, 0xad, 0xde,
	0x8C, 0x0A, 0x0a, 0x00
};

const static char changecct_answer_node[] = {
	0x12, 0x00, 0x01, 0x33, 0x04, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x00,
	0x78, 0x56, 0x34, 0x12, 0xef, 0xbe, 0xad, 0xde,
	0x00
};

const static char setrgbw_query_node[] = {
	0x14, 0x00, 0x00, 0x36, 0x05, 0x00, 0x00, 0x00,
	0x78, 0x56, 0x34, 0x12, 0xef, 0xbe, 0xad, 0xde,
	0x01, 0x02, 0x03, 0x04, 0x0a, 0x00
};

const static char setrgbw_answer_node[] = {
	0x12, 0x00, 0x01, 0x36, 0x05, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x00,
	0x78, 0x56, 0x34, 0x12, 0xef, 0xbe, 0xad, 0xde,
	0x00
};

const static char setbright_query_node[] = {
	0x11, 0x00, 0x00, 0x31, 0x06, 0x00, 0x00, 0x00,
	0x78, 0x56, 0x34, 0x12, 0xef, 0xbe, 0xad, 0xde,
	0x12,  0x0a, 0x00
};

const static char setbright_answer_node[] = {
	0x12, 0x00, 0x01, 0x31, 0x06, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x00,
	0x78, 0x56, 0x34, 0x12, 0xef, 0xbe, 0xad, 0xde,
	0x00
};

const static char requpdate_query_node[] = {
	0x0e, 0x00, 0x00, 0x68, 0x07, 0x00, 0x00, 0x00,
	0x78, 0x56, 0x34, 0x12, 0xef, 0xbe, 0xad, 0xde,
};

const static char requpdate_answer_node[] = {
	0x1d, 0x00, 0x00, 0x68, 0x07, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x00,
	0x78, 0x56, 0x34, 0x12, 0xef, 0xbe, 0xad, 0xde,
	0x02,
	0x00, 0x01, 0x55, 0x8C, 0x0A, 0x10, 0x11, 0x12, 0x13
};


const static char req_getgroups[] = {
	0x07, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x35
};

const static char req_getgroups_answer[] = {
 0x3f, 0x00, 0x01, 0x1e, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x03, 0x00,
 0x01, 0x00, 0x47, 0x72, 0x75, 0x70, 0x70, 0x65,
 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00,
 0x02, 0x00, 0x47, 0x72, 0x75, 0x70, 0x70, 0x65,
 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00,
 0x03, 0x00, 0x47, 0x72, 0x75, 0x70, 0x70, 0x65,
 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00
};

void setup(void) {
	int err = lightify_new(&_ctx, NULL);
	printf("setup done\n");
}

void teardown(void) {
	if (_ctx)
		lightify_free(_ctx);
	printf("teardown done\n");
	_ctx = NULL;
}

// short helper to setup our answer and free stuff if necessary.
static void helper_mfs_setup_answer(struct fake_socket *mfs,
		const char *our_answer, size_t answer_len) {
	if (mfs->buf_read)
		free(mfs->buf_read);
	mfs->buf_read = malloc(answer_len);
	memcpy(mfs->buf_read, our_answer, answer_len);
	mfs->size_read = answer_len;
	if (mfs->buf_write)
		free(mfs->buf_write);
	mfs->buf_write = NULL;
	mfs->size_write = 0;
}

START_TEST(lightify_context_base_NULL_checks)
	{

		// tests ABI default behaviour when supplying a NULL pointer to the context

		printf("now testing context::userdata\n");
		ck_assert_int_eq(lightify_get_userdata(NULL), NULL);
		ck_assert_int_eq(lightify_set_userdata(NULL,0), -EINVAL);

		printf("now testing context::log\n");
		ck_assert_int_eq(lightify_set_log_priority(NULL,0), -EINVAL);
		ck_assert_int_eq(lightify_get_log_priority(NULL), -EINVAL);
		ck_assert_int_eq(lightify_set_log_fn(NULL,NULL), -EINVAL);

		printf("now testing context::socket\n");
		ck_assert_int_eq(lightify_set_socket_fn(NULL,NULL,NULL), -EINVAL);
		//ck_assert_int_eq(lightify_set_socket_fn((void*)1,(void*)1,NULL), -EINVAL);
		//ck_assert_int_eq(lightify_set_socket_fn((void*)1,NULL,(void*)1), -EINVAL);

		printf("now testing context::context\n");
		ck_assert_int_eq(lightify_free(NULL), -EINVAL);

		printf("now testing context::nodes\n");
		ck_assert_int_eq(lightify_get_next_node(NULL,NULL), NULL);

	}END_TEST

START_TEST(lightify_context_object)
	{
		// testing obtaining an ctx object
		struct lightify_ctx *ctx;

		printf("now testing context\n");
		// creation of the object.
		int err = lightify_new(&ctx, NULL);
		ck_assert_int_eq(err, 0);
		ck_assert_ptr_ne(ctx, 0);

		ck_assert_int_eq(lightify_free(ctx), 0);
	}END_TEST

Suite *liblightify_testfixture(void) {
	Suite *s;
	TCase *tc_context;

	s = suite_create("liblithify_ABI");
	tc_context = tcase_create("testfixture basic tests");
	tcase_add_test(tc_context, lightify_context_object);
	suite_add_tcase(s, tc_context);
	return s;
}

START_TEST(lightify_context_userdata)
	{
		// userdata default value and if it is settable
		struct lightify_ctx *ctx;
		int err = lightify_new(&ctx, NULL);

		ck_assert_ptr_eq(lightify_get_userdata(ctx), 0);
		lightify_set_userdata(ctx, (void*) 42);
		ck_assert_ptr_eq(lightify_get_userdata(ctx), (void* )42);

	}END_TEST

START_TEST(lightify_context_skt)
	{

		struct lightify_ctx *ctx;
		int err = lightify_new(&ctx, NULL);
		// default fd not set, so should error out
		err = lightify_skt_getfd(ctx);
		ck_assert((err < 0));

		// is it setable and retains its value?
		err = lightify_skt_setfd(ctx, 1);
		err = lightify_skt_getfd(ctx);
		ck_assert_int_eq(err, 1);
	}END_TEST

Suite *liblightify_API_suite(void) {
	Suite *s;
	TCase *tc;
	s = suite_create("liblightify");

	/* Core test case */
	tc = tcase_create("API_misuse");

	tcase_add_test(tc, lightify_context_base_NULL_checks);
	suite_add_tcase(s, tc);

	tc = tcase_create("context_API_check");
	tcase_add_test(tc, lightify_context_userdata);
	suite_add_tcase(s, tc);

	tc = tcase_create("context_skt_API_check");
	tcase_add_test(tc, lightify_context_skt);
	suite_add_tcase(s, tc);

	return s;
}

int my_write_to_socket(struct lightify_ctx *ctx, unsigned char *msg,
		size_t size) {

	struct fake_socket *fs = (struct fake_socket*) lightify_get_userdata(ctx);
	assert(fs);

	printf("my_write_from_socket: size=%d, msg=%p\n", size, msg);

	if (!fs->buf_write) {
		fs->buf_write = malloc(size);
		fs->size_write = 0;
	} else {
		fs->buf_write = realloc(fs->buf_write, fs->size_write + size);
	}
	assert(fs->buf_write);

	memcpy(fs->buf_write + fs->size_write, msg, size);
	fs->size_write += size;

	return (fs->err_write < 0 ? fs->err_write : size);
}

int my_read_from_socket(struct lightify_ctx *ctx, unsigned char *msg,
		size_t size) {

	size_t read_size;
	struct fake_socket *fs = (struct fake_socket*) lightify_get_userdata(ctx);
	assert(fs);
	assert(fs->buf_read);

	printf("my_read_from_socket: size=%d, msg=%p\n", size, msg);

	read_size = fs->size_read;
	if (read_size > size)
		read_size = size;

	if (read_size) {
		memcpy(msg, fs->buf_read, read_size);
		fs->size_read -= read_size;
		if (fs->size_read)
			memmove(fs->buf_read, fs->buf_read + read_size, fs->size_read);
	}

	return (fs->err_read < 0 ? fs->err_read : read_size);
}

START_TEST(lightify_tst_scan_nodes)
	{
		// scan for nodes using static data
		// tests also the overriding of the io functions :)

		int err;
		struct fake_socket *mfs = calloc(1, sizeof(struct fake_socket));
		helper_mfs_setup_answer(mfs, scanfornodes_answer,
				sizeof(scanfornodes_answer));

		err = lightify_set_socket_fn(_ctx, my_write_to_socket,
				my_read_from_socket);
		ck_assert_int_eq(err, 0);

		err = lightify_set_userdata(_ctx, mfs);
		ck_assert_int_eq(err, 0);

		err = lightify_scan_nodes(_ctx);
		ck_assert_int_eq(err, 1);

		ck_assert_int_eq(mfs->size_write, sizeof(scanfornodes_query));
		err = memcmp(mfs->buf_write, scanfornodes_query, mfs->size_write);
		ck_assert_int_eq(err, 0);

		// check if we can access the node
		struct lightify_node *node, *node2;

		node = lightify_get_next_node(_ctx, NULL);
		ck_assert_ptr_ne(node, NULL);

		node2 = lightify_get_next_node(_ctx, node);
		ck_assert_ptr_eq(node2, NULL);

		// check if we can get the name
		const char *name = lightify_node_get_name(node);
		ck_assert_ptr_ne(name, NULL);
		ck_assert_int_eq(strcmp("Licht 01", name), 0);

		// check if we can get the CCT
		int cct = lightify_node_get_cct(node);
		ck_assert_int_eq(cct, 2702);

		// check if we can get the MAC
		uint64_t mac = lightify_node_get_nodeadr(node);
		ck_assert(mac == 0xdeadbeef12345678);

		// check that we can search via MAC.
		node2 = lightify_get_nodefrommac(_ctx, mac);
		ck_assert_ptr_eq(node,node2);

		// check that we won't get a result on a bogus MAC.
		node2 = lightify_get_nodefrommac(_ctx, mac+1);
		ck_assert_ptr_eq(node2,NULL);

		// check if we can get the node adr
		uint16_t zoneadr = lightify_node_get_zoneadr(node);
		ck_assert_uint_eq(zoneadr, 0xaa55);

		// check if we can extract the group adr
		uint16_t grp = lightify_node_get_grpadr(node);
		ck_assert_uint_eq(grp, 0xabcd);

		// check if we can get the colors
		uint8_t color;

		color = lightify_node_get_red(node);
		ck_assert_int_eq(color, 0xf0);

		color = lightify_node_get_green(node);
		ck_assert_int_eq(color, 0xf1);

		color = lightify_node_get_blue(node);
		ck_assert_int_eq(color, 0xf2);

		color = lightify_node_get_white(node);
		ck_assert_int_eq(color, 0xf3);

		// Check we delete node information when scanfornodes fails.
		// we do that by repeating the scan, but with the now invalid token,
		// so we should get EPROTO here.
		helper_mfs_setup_answer(mfs, scanfornodes_answer,
				sizeof(scanfornodes_answer));

		err = lightify_scan_nodes(_ctx);
		ck_assert_int_eq(err, -EPROTO);

		// There must be no node.
		node = lightify_get_next_node(_ctx,NULL);
		ck_assert_ptr_eq(node, NULL);

		// undo functions and userdata.
		err = lightify_set_socket_fn(_ctx, NULL, NULL);
		ck_assert_int_eq(err, 0);

		err = lightify_set_userdata(_ctx, NULL);
		ck_assert_int_eq(err, 0);

		free(mfs->buf_write);
		free(mfs->buf_read);
		free(mfs);
	}END_TEST

Suite *liblightify_functional_nodes(void) {
	Suite *s;
	TCase *tc;
	s = suite_create("liblightify_functional_nodes");

	/* Core test case */
	tc = tcase_create("lightify_tst_scan_nodes");

	tcase_add_unchecked_fixture(tc, setup, teardown);
	tcase_add_test(tc, lightify_tst_scan_nodes);
	suite_add_tcase(s, tc);

	return s;
}

START_TEST(lightify_tst_manipulate_nodes)
	{

		// setup: scan for nodes with static data as response (to setup nodes)
		// and then issue commands at it to compare with static result data.

		/* setup part -- already checked in another testcase */
		int err;
		struct fake_socket *mfs = calloc(1, sizeof(struct fake_socket));
		helper_mfs_setup_answer(mfs, scanfornodes_answer,
				sizeof(scanfornodes_answer));

		lightify_set_socket_fn(_ctx, my_write_to_socket, my_read_from_socket);
		lightify_set_userdata(_ctx, mfs);
		err = lightify_scan_nodes(_ctx);
		ck_assert_int_ge(err, 0);

		// test: turn on light, broadcast
		do {
			helper_mfs_setup_answer(mfs, turnonlight_answer_broadcast,
					sizeof(turnonlight_answer_broadcast));

			// test: turn on light, broadcast
			err = lightify_request_set_onoff(_ctx, NULL, 1);
			ck_assert_int_eq(err, 0);
			err = memcmp(mfs->buf_write, turnonlight_query_broadcast,
					mfs->size_write);
			ck_assert_int_eq(err, 0);

			// test: turn on light, addessing node
			helper_mfs_setup_answer(mfs, turnonlight_answer_node,
					sizeof(turnonlight_answer_node));
			err = lightify_request_set_onoff(_ctx,
					lightify_get_next_node(_ctx, NULL), 1);
			ck_assert_int_eq(err, 0);

			err = memcmp(mfs->buf_write, turnonlight_query_node,
					mfs->size_write);
			if (err) {
				char buf[512];
				buf[0] = '\n';
				buf[1] = '0';

				int i = 0;
				for (i = 0; i < mfs->size_write; i++) {
					unsigned int one, two;
					one = 0xff & mfs->buf_write[i];
					two = 0xff & turnonlight_query_node[i];
					sprintf(buf + strlen(buf), " %02x%c=%02x", one,
							(one == two ? '=' : '!'), two);
				}
				ck_abort_msg(buf);
			}
			ck_assert_int_eq(err, 0);

			// check if cache has been updated.
			err = lightify_node_is_on(lightify_get_next_node(_ctx,0));
			ck_assert_int_eq(err, 1);

		} while(0);

		// Test change CCT.
		do {
			helper_mfs_setup_answer(mfs, changecct_answer_node,
					sizeof(changecct_answer_node));
			err = lightify_request_set_cct(_ctx, lightify_get_next_node(_ctx, NULL),
					2700, 10);
			ck_assert_int_eq(err, 0);
			err = memcmp(mfs->buf_write, changecct_query_node, mfs->size_write);
			if (err) {
				char buf[512];
				buf[0] = '\n';
				buf[1] = '0';

				int i = 0;
				for (i = 0; i < mfs->size_write; i++) {
					unsigned int one, two;
					one = 0xff & mfs->buf_write[i];
					two = 0xff & changecct_query_node[i];
					sprintf(buf + strlen(buf), " %02x%c=%02x", one,
							(one == two ? '=' : '!'), two);
				}
				ck_abort_msg(buf);
			}
			ck_assert_int_eq(err, 0);

			// Check if the cache has been updated
			err = lightify_node_get_cct(lightify_get_next_node(_ctx,0));
			ck_assert_int_eq(err, 2700);

		} while(0);

		// Test RBGW
		do {
			helper_mfs_setup_answer(mfs, setrgbw_answer_node,
					sizeof(setrgbw_answer_node));
			err = lightify_request_set_rgbw(_ctx, lightify_get_next_node(_ctx, NULL),
					1,2,3,4,10);
			ck_assert_int_eq(err, 0);
			err = memcmp(mfs->buf_write, setrgbw_query_node, mfs->size_write);
			if (err) {
				char buf[512];
				buf[0] = '\n';
				buf[1] = '0';

				int i = 0;
				for (i = 0; i < mfs->size_write; i++) {
					unsigned int one, two;
					one = 0xff & mfs->buf_write[i];
					two = 0xff & setrgbw_query_node[i];
					sprintf(buf + strlen(buf), " %02x%c=%02x", one,
							(one == two ? '=' : '!'), two);
				}
				ck_abort_msg(buf);
			}
			ck_assert_int_eq(err, 0);
			// Check if the cache has been updated
			err = lightify_node_get_red(lightify_get_next_node(_ctx,0));
			ck_assert_int_eq(err, 1);
			err = lightify_node_get_green(lightify_get_next_node(_ctx,0));
			ck_assert_int_eq(err, 2);
			err = lightify_node_get_blue(lightify_get_next_node(_ctx,0));
			ck_assert_int_eq(err, 3);
			err = lightify_node_get_white(lightify_get_next_node(_ctx,0));
			ck_assert_int_eq(err, 4);
		} while (0);

		// Test brightness
		do {
			helper_mfs_setup_answer(mfs, setbright_answer_node,
					sizeof(setbright_answer_node));
			err = lightify_request_set_brightness(_ctx, lightify_get_next_node(_ctx, NULL),
					0x12,10);
			ck_assert_int_eq(err, 0);
			err = memcmp(mfs->buf_write, setbright_query_node, mfs->size_write);
			if (err) {
				char buf[512];
				buf[0] = '\n';
				buf[1] = '0';

				int i = 0;
				for (i = 0; i < mfs->size_write; i++) {
					unsigned int one, two;
					one = 0xff & mfs->buf_write[i];
					two = 0xff & setbright_query_node[i];
					sprintf(buf + strlen(buf), " %02x%c=%02x", one,
							(one == two ? '=' : '!'), two);
				}
				ck_abort_msg(buf);
			}
			ck_assert_int_eq(err, 0);
			// Check if the cache has been updated
			err = lightify_node_get_brightness(lightify_get_next_node(_ctx,0));
			ck_assert_int_eq(err, 0x12);
		} while (0);

		// Test updating node information
		do {
		helper_mfs_setup_answer(mfs, requpdate_answer_node,
				sizeof(requpdate_answer_node));
		err = lightify_request_update_node(_ctx, lightify_get_next_node(_ctx, NULL));
		ck_assert_int_eq(err, 0);

		err = memcmp(mfs->buf_write, requpdate_query_node, mfs->size_write);
		if (err) {
			char buf[512];
			buf[0] = '\n';
			buf[1] = '0';

			int i = 0;
			for (i = 0; i < mfs->size_write; i++) {
				unsigned int one, two;
				one = 0xff & mfs->buf_write[i];
				two = 0xff & setbright_query_node[i];
				sprintf(buf + strlen(buf), " %02x%c=%02x", one,
						(one == two ? '=' : '!'), two);
			}
			ck_abort_msg(buf);
		}
		ck_assert_int_eq(err, 0);

			// Check if the cache has been updated
			struct lightify_node* node = lightify_get_next_node(_ctx, NULL);
			ck_assert_int_eq(lightify_node_get_onlinestate(node),0);
			ck_assert_int_eq(lightify_node_is_on(node),0x01);
			ck_assert_int_eq(lightify_node_get_brightness(node), 0x55);
			ck_assert_int_eq(lightify_node_get_cct(node),0xa8c);
			ck_assert_int_eq(lightify_node_get_red(node),0x10);
			ck_assert_int_eq(lightify_node_get_green(node),0x11);
			ck_assert_int_eq(lightify_node_get_blue(node),0x12);
			ck_assert_int_eq(lightify_node_get_white(node),0x13);
		} while (0);

	}END_TEST

Suite *liblightify_functional_manipulate_node(void) {
	Suite *s;
	TCase *tc;
	s = suite_create("liblightify_functional_manipulate_node");

	/* Core test case */
	tc = tcase_create("liblightify_functional_manipulate_node");

	tcase_add_unchecked_fixture(tc, setup, teardown);
	tcase_add_test(tc, lightify_tst_manipulate_nodes);
	suite_add_tcase(s, tc);

	return s;
}

int main(void) {
	int number_failed;
	Suite *s;
	SRunner *sr;

	sr = srunner_create(liblightify_testfixture());
	srunner_add_suite(sr, liblightify_API_suite());
	srunner_add_suite(sr, liblightify_functional_nodes());
	srunner_add_suite(sr, liblightify_functional_manipulate_node());

	srunner_set_tap(sr, "-");
	srunner_set_fork_status(sr, CK_NOFORK);
	srunner_run_all(sr, CK_VERBOSE);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
