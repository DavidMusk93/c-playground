#!/bin/bash

set -ex
(
    cd $(dirname $0)/cmake-build-debug
    rm * -rf
    cmake .. && make
)
