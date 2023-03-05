#!/usr/bin/env bash

set -eux

cd $(dirname $0)

rm -rf zstd-tmp
git clone -b v1.5.4 https://github.com/facebook/zstd zstd-tmp
cd zstd-tmp/build/single_file_libs
./create_single_file_library.sh
cd ../../..
mv zstd-tmp/build/single_file_libs/zstd.c zstd/zstd.c
mv zstd-tmp/lib/zstd.h zstd/zstd.h
mv zstd-tmp/COPYING zstd/COPYING
rm -rf zstd-tmp
