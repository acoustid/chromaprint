#!/usr/bin/env bash

set -eux

cd $(dirname $0)/../..
git subtree pull --squash --prefix src/3rdparty/kissfft https://github.com/mborgerding/kissfft v131
