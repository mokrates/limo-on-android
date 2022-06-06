#!/bin/bash

if [ -z "$1" ]; then
    echo "usage: $0 -y"
    echo "fetch the boehm collector library which is not included in this repo"
else
    pushd $(dirname "$0")
    BASEDIR=$(pwd)
    wget -c 'https://www.hboehm.info/gc/gc_source/gc-8.0.6.tar.gz'
    wget -c 'https://www.hboehm.info/gc/gc_source/libatomic_ops-7.6.10.tar.gz'
    pushd ..
    tar xvzf stub-gc-8.0.6/gc-8.0.6.tar.gz
    pushd gc-8.0.6
    tar xvzf ../stub-gc-8.0.6/libatomic_ops-7.6.10.tar.gz
    mv libatomic_ops-7.6.10 libatomic_ops
    patch CMakeLists.txt ../stub-gc-8.0.6/CMakeLists.txt.patch 
    popd
    popd
    popd
fi
