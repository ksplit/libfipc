#!/bin/sh

aclocal -I m4 && \
    autoheader && \
    automake --gnu --add-missing && \
    autoconf
