#!/bin/bash

set -x

limo ~/limo/examples/make_module_dict.limo program.limo > _program.limo
xxd -i _program.limo > _program.h
