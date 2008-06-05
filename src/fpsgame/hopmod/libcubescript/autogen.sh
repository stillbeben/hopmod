#!/bin/sh

aclocal -I /usr/local/share/aclocal
automake -a -c --foreign
autoconf
