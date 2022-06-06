#!/bin/bash

USAGE=$(echo "usage $0" '{ prep_deps | make_program | realclean }')

if [ -z "$1" ]; then
    echo $USAGE
elif [ "$1" == "prep_deps" ]; then
    bash stub-limo/getlib.sh -y
    bash stub-gmp/getlib.sh -y
    bash stub-gc-8.0.6/getlib.sh -y
elif [ "$1" == "make_program" ]; then
    shift
    bash make_program.sh "$@"
elif [ "$1" == "realclean" ]; then
    rm -f _program.*
    rm -rf gc-8.0.6
    rm -rf gmp
    rm -rf limo
    find . -name '*~' -delete
else
    echo $USAGE
fi
