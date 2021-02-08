#!/bin/bash

if [ -z "$MAKEFLAGS" ]
then
      echo "please set MAKEFLAGS, see https://github.com/beaumanvienna/marley#compile-from-source"
      echo "aborting..."
      exit 1
fi

aclocal && autoconf && automake --add-missing --foreign && ./configure --prefix=/usr && make
