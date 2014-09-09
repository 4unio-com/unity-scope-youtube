#!/bin/sh

# Copyright (C) 2014 Canonical Ltd
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authored by: Pete Woods <pete.woods@canonical.com>

# Simple script to build a click packaged scope.

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 armhf|amd64|i386"
    exit 1
fi

CLICK_ARCH="$1"
BUILDDIR="$PWD/builddir"

rm -rf "${BUILDDIR}"
mkdir -p "${BUILDDIR}/build"

(
    cd "${BUILDDIR}/build"
    eval "
    click chroot \
        -a ${CLICK_ARCH} -f ubuntu-sdk-14.10 -s utopic run \
    cmake ../.. \
       -DCMAKE_INSTALL_PREFIX:PATH='${BUILDDIR}/click' \
        -DCLICK_MODE=on \
        -DCLICK_ARCH=${CLICK_ARCH}
    "
    click chroot \
        -a ${CLICK_ARCH} -f ubuntu-sdk-14.10 -s utopic run \
    make install
)

click build "${BUILDDIR}/click"
rm -rf "${BUILDDIR}"
