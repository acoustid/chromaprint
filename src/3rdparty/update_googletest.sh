#!/usr/bin/env bash

set -eux

cd $(dirname $0)/../..
git subtree pull --squash --prefix src/3rdparty/googletest https://github.com/google/googletest.git v1.16.0
