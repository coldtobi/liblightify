EXTRA_DIST = doc/protocol.txt Doxyfile
CLEANFILES =
ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AM_MAKEFLAGS = --no-print-directory

clean-local:
	-rm -rf doc/doxygen/

include $(top_srcdir)/doxygen-include.am

AM_CPPFLAGS = \
	-include $(top_builddir)/config.h \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-I${top_srcdir}/src

AM_CFLAGS = ${my_CFLAGS} \
	-fvisibility=hidden \
	-ffunction-sections \
	-fdata-sections

AM_LDFLAGS = \
	-Wl,--gc-sections \
	-Wl,--as-needed \
	${LDFLAG_NO_UNDEFINED}

SED_PROCESS = \
	$(AM_V_GEN)$(MKDIR_P) $(dir $@) && $(SED) \
	-e 's,@VERSION\@,$(VERSION),g' \
	-e 's,@prefix\@,$(prefix),g' \
	-e 's,@exec_prefix\@,$(exec_prefix),g' \
	-e 's,@libdir\@,$(libdir),g' \
	-e 's,@includedir\@,$(includedir),g' \
	< $< > $@ || rm $@

%.pc: %.pc.in Makefile
	$(SED_PROCESS)

LIBLIGHTIFY_CURRENT=0
LIBLIGHTIFY_REVISION=0
LIBLIGHTIFY_AGE=0

pkginclude_HEADERS = src/liblightify/liblightify.h src/liblightify++/liblightify++.hpp
lib_LTLIBRARIES = src/liblightify.la

src_liblightify_la_SOURCES =\
	src/liblightify-private.h \
	src/liblightify.c \
	src/log.c \
	src/log.h \
	src/context.c \
	src/context.h \
	src/node.c \
	src/node.h \
	src/groups.c \
	src/groups.h \
	src/socket.c \
	src/socket.h

EXTRA_DIST += src/liblightify.sym autogen.sh ChangeLog TODO

src_liblightify_la_LDFLAGS = $(AM_LDFLAGS) \
	-version-info $(LIBLIGHTIFY_CURRENT):$(LIBLIGHTIFY_REVISION):$(LIBLIGHTIFY_AGE) \
	-Wl,--version-script=$(top_srcdir)/src/liblightify.sym
src_liblightify_la_DEPENDENCIES = ${top_srcdir}/src/liblightify.sym

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = src/liblightify.pc
EXTRA_DIST += src/liblightify.pc.in
CLEANFILES += src/liblightify.pc

#AM_TESTS_FD_REDIRECT = 9>&2
AM_TESTS_ENVIRONMENT = \
	export CK_TAP_LOG_FILE_NAME="-";

TEST_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
                  $(top_srcdir)/build-aux/tap-driver.sh

TESTS = src/test-lightify

check_PROGRAMS = src/test-lightify
src_test_lightify_SOURCES = src/tests/test-lightify.c
src_test_lightify_CFLAGS = @CHECK_CFLAGS@ -I $(top_srcdir)/src/liblightify/
src_test_lightify_LDADD = $(top_builddir)/src/liblightify.la @CHECK_LIBS@


bin_PROGRAMS = src/tools/lightify-util src/tools/lightify-example
src_tools_lightify_util_SOURCES = src/tools/lightify-util.c
src_tools_lightify_util_CFLAGS = @CHECK_CFLAGS@ -I $(top_srcdir)/src/liblightify/
src_tools_lightify_util_LDADD = $(top_builddir)/src/liblightify.la @CHECK_LIBS@

src_tools_lightify_example_SOURCES = src/tools/lightify-example.cpp
src_tools_lightify_example_CXXFLAGS = @CHECK_CFLAGS@ -I $(top_srcdir)/src/liblightify/
src_tools_lightify_example_LDADD = $(top_builddir)/src/liblightify.la @CHECK_LIBS@
