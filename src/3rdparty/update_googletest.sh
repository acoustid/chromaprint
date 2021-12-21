#!/usr/bin/env bash

set -eux

cd $(dirname $0)/../..
git subtree pull --squash --prefix src/3rdparty/googletest https://github.com/google/googletest.git release-1.11.0
