#!/usr/bin/env bash

set -eux

cd $(dirname $0)/../..
git subtree pull --squash --prefix src/3rdparty/zstd https://github.com/facebook/zstd v1.5.4
