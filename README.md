# liblightify
Library to control OSRAM Lightify

## Building

The libary uses autotools. To build it use

./autogen.sh && ./configure && make

## Dependencies

The doxygen integration into autoconf needs a recent ax_prog_doxygen.m4.
The required changes have been commited to the autoconf-archive 2015-11-23,
so any autoconf-archive beyond this should work.
See https://www.gnu.org/software/autoconf-archive/ax_prog_doxygen.html

The testsuite needs "check"  -- check.sourceforge.net

## Usage

In src/tools are two small examples how to use the library. This should get you started.
Look at the header liblightify.h and liblightify++.h for the API, there is some doxygen documentation.
