#!/bin/bash

DESTDIR=$(pwd)
pushd $1
xxd -i dosbox-8x16.fnt > $DESTDIR/dosbox-8x16.c
popd
