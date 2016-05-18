AC_PREREQ(2.60)
AC_INIT([lightify],
        [0],
        [tobi@coldtobi.de],
        [liblightify],
        [https://github.com/coldtobi/liblightify])
AC_CONFIG_SRCDIR([src/liblightify.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_REQUIRE_AUX_FILE([tap-driver.sh])

AM_INIT_AUTOMAKE([
	check-news
	foreign
	1.11
	-Wall
	-Wno-portability
	silent-rules
	tar-pax
	no-dist-gzip
	dist-xz
	subdir-objects
])

AC_PROG_CC
AC_PROG_CXX

# Doxygen
DX_DOXYGEN_FEATURE(ON)
DX_DOT_FEATURE(ON)
DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_PS_FEATURE(OFF)
DX_INIT_DOXYGEN([$PACKAGE_NAME],[Doxyfile],[doc/doxygen])

#AC_USE_SYSTEM_EXTENSIONS
#AM_PATH_CHECK

PKG_CHECK_MODULES([CHECK], [check >= 0.10.0])

AC_PROG_CC_STDC
AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE
AC_CONFIG_MACRO_DIR([m4])

# Debug messages are only emitted when logging is enabled (default)
# and verbosity is set accordingly.
# verbosity can be set via the enviorment by exporting lightify_LOG
# to "err", "info" and "debug".
# alternatively, verbosity can be adjusted by the API: see lightify_set_log_priority()

# Some extra verbose debug messages, which goes over the ENABLE_DEBUG below
# (For example, enables protocol dump when using standard I/O handlers)
AC_ARG_ENABLE([debugmsg],
    AS_HELP_STRING([--enable-debugmsg],
      [build with additional debugging messages. Do not use for production mode.@<:@default=disabled@:>@]),
      enable_debugmsg=yes)
AS_IF([test "x$enable_debugmsg" = "xyes"], [
        AC_DEFINE(ENABLE_DEBUG_MSGS, [1], [Extra debug messages.])
])

# Some extra verbose messages, emitted during protocol checks. Aims to help debugging why some message
# or answer has been rejected.
AC_ARG_ENABLE([debugprotocol],
    AS_HELP_STRING([--enable-debugprotocol],
      [build with very verbose protocol debugging. Do not use for production mode.@<:@default=disabled@:>@]),
      enable_debugprotocol=yes)
AS_IF([test "x$enable_debugprotocol" = "xyes"], [
        AC_DEFINE(ENABLE_DEBUG_PROTO, [1], [Extra verbose protocol debug messages.])
])

# General debugging messages, not in the above criterias.
AC_ARG_ENABLE([debug],
        AS_HELP_STRING([--enable-debug], [enable debug messages @<:@default=disabled@:>@]),
        [], [enable_debug=no])
AS_IF([test "x$enable_debug" = "xyes"], [
        AC_DEFINE(ENABLE_DEBUG, [1], [Debug messages.])
])



LT_INIT([
	disable-static
	pic-only
])
AC_PREFIX_DEFAULT([/usr])

AC_PROG_SED
AC_PROG_MKDIR_P

AC_ARG_ENABLE([logging],
        AS_HELP_STRING([--disable-logging], [disable system logging @<:@default=enabled@:>@]),
        [], enable_logging=yes)
AS_IF([test "x$enable_logging" = "xyes"], [
        AC_DEFINE(ENABLE_LOGGING, [1], [System logging.])
])

AC_CHECK_FUNCS([ \
	__secure_getenv \
	secure_getenv\
])

my_CFLAGS="\
-Wall \
-Wchar-subscripts \
-Wformat-security \
-Wmissing-declarations \
-Wmissing-prototypes \
-Wnested-externs \
-Wpointer-arith \
-Wshadow \
-Wsign-compare \
-Wstrict-prototypes \
-Wtype-limits \
"

AC_CANONICAL_HOST

if test "$host_os" = cygwin
then
    LDFLAG_NO_UNDEFINED="-no-undefined"
fi
AC_SUBST([LDFLAG_NO_UNDEFINED])


AC_SUBST([my_CFLAGS])

AC_CONFIG_HEADERS(config.h)
AC_CONFIG_FILES([
        Makefile
])

AC_OUTPUT
AC_MSG_RESULT([
        $PACKAGE $VERSION
        =====

        prefix:                 ${prefix}
        sysconfdir:             ${sysconfdir}
        libdir:                 ${libdir}
        includedir:             ${includedir}

        compiler:               ${CC}
        cflags:                 ${CFLAGS}
        ldflags:                ${LDFLAGS}

        logging:                ${enable_logging}
        debug:                  ${enable_debug}
])
