#!/bin/bash

(
    cd $(dirname $0)
    BUILDDIR=BUILD
    mkdir -p $BUILDDIR
    cd $BUILDDIR && rm -rf * && cmake .. && make && make install
)
