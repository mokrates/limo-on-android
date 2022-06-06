#!/bin/bash

if [ -z "$1" ]; then
    echo "usage: $0 -y"
    echo "fetch the compiled GMP library which is not included in this repo"
else
    pushd $(dirname "$0")
    pushd ..
    git clone https://github.com/Rupan/gmp
    popd
    popd
fi
