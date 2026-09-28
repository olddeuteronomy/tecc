#!/bin/bash

# `smolbsd-build.sh SERVICE'
# ==========================

# NETBSD_ROOT envvar should point to the smolBSD directory.
if [ -z "$NETBSD_ROOT" ]; then
    echo "Error: NETBSD_ROOT envvar not defined."
    exit 2
fi

if [ -z "$1" ]; then
    echo "Usage: smolbsd-build.sh SERVICE"
    exit 1
fi

pushd .
IMAGE="smolerfiles/Dockerfile.$1"
cd $NETBSD_ROOT
echo "Building $IMAGE ..."
./smoler.sh build $IMAGE
popd
