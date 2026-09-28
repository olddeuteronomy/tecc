#!/bin/bash

# `smolbsd-run.sh SERVICE'
# ========================

# NETBSD_ROOT envvar should point to the smolBSD directory.
if [ -z "$NETBSD_ROOT" ]; then
    echo "Error: NETBSD_ROOT envvar not defined."
    exit 2
fi

if [ -z "$1" ]; then
    echo "Usage: smolbsd-run.sh SERVICE"
    exit 1
fi

ARCH="amd64"
VERS="latest"
IMAGE="images/$1-${ARCH}\:${VERS}.img"

pushd .
cd $NETBSD_ROOT
# 1) -i IMAGE       use raw VM IMAGE
# 2) -P             use PTY terminal
# 3) -n 2           enable stopping VM from ksh (as `su'): . /etc/include/shutdown
# 4) -w PATH        mount host's PATH to guest's /mnt/
# ./startnb.sh -i "${IMAGE}" -P -n 2 -w ${HOME}/workspace/devel
./startnb.sh -f "etc/$1.conf" -P -n 2 -w ${HOME}/workspace/devel
popd
