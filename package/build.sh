#!/usr/bin/env bash

: ${OS?}
: ${ARCH?}

set -eux

BASE_DIR=$(cd $(dirname $0)/.. && pwd)

TMP_BUILD_DIR=$(mktemp -d -p $BASE_DIR build.XXXXXXXX)
trap 'rm -rf $TMP_BUILD_DIR' EXIT

cd $TMP_BUILD_DIR

wget -q -O artifacts.zip "https://code.oxygene.sk/acoustid/ffmpeg-build/builds/artifacts/master/download?job=build%3A$OS-$ARCH"
unzip artifacts.zip
export FFMPEG_DIR=$TMP_BUILD_DIR/$(ls -d ffmpeg-* | tail)

CMAKE_ARGS=(
    -DCMAKE_INSTALL_PREFIX=$BASE_DIR/chromaprint-$OS-$ARCH
    -DCMAKE_BUILD_TYPE=Release
    -DBUILD_TOOLS=ON
    -DBUILD_TESTS=OFF
    -DBUILD_SHARED_LIBS=OFF
)

STRIP=strip

case $OS in
windows)
    perl -pe "s!{EXTRA_PATHS}!$FFMPEG_DIR!g" $BASE_DIR/package/toolchain-mingw.cmake.in | perl -pe "s!{ARCH}!$ARCH!g" >toolchain.cmake
    CMAKE_ARGS+=(
        -DCMAKE_TOOLCHAIN_FILE=$TMP_BUILD_DIR/toolchain.cmake
        -DCMAKE_C_FLAGS='-static -static-libgcc -static-libstdc++'
        -DCMAKE_CXX_FLAGS='-static -static-libgcc -static-libstdc++'
    )
    STRIP=$ARCH-w64-mingw32-strip
    ;;
linux)
    case $ARCH in
    i686)
        export CC='gcc -m32'
        export CXX='g++ -m32'
        ;;
    x86_64)
        ;;
    *)
        echo "unsupported architecture ($ARCH)"
        exit 1
    esac
    ;;
*)
    echo "unsupported OS ($OS)"
    exit 1
    ;;
esac

cmake ${CMAKE_ARGS[@]} $BASE_DIR

make
make install

$STRIP $BASE_DIR/chromaprint-$OS-$ARCH/bin/*
