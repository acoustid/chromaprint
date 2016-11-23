#!/usr/bin/env bash

: ${OS?}
: ${ARCH?}

set -eux

BASE_DIR=$(cd $(dirname $0)/.. && pwd)

TMP_BUILD_DIR=$(mktemp -d -p $BASE_DIR build.XXXXXXXX)
trap 'rm -rf $TMP_BUILD_DIR' EXIT

cd $TMP_BUILD_DIR

wget -q -O artifacts.zip "https://code.oxygene.sk/acoustid/ffmpeg-build/builds/artifacts/master/download?job=$OS+$ARCH"
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

OSX_CC=/usr/local/bin/gcc-5
OSX_SDK=/Developer/SDKs/MacOSX10.4u.sdk
OSX_VERSION=10.4

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
macos)
    CMAKE_ARGS+=(
        -DCMAKE_TOOLCHAIN_FILE=$TMP_BUILD_DIR/toolchain.cmake
        -DCMAKE_C_COMPILER=$OSX_CC
        -DCMAKE_CXX_COMPILER=$OSX_CXX
        -DCMAKE_OSX_SYSROOT=$OSX_SDK
        -DCMAKE_OSX_DEPLOYMENT_TARGET=$OSX_VERSION
        -DCMAKE_OSX_ARCHITECTURES=$ARCH
    )
    ;;
linux)
    case $ARCH in
    i686)
        CMAKE_ARGS+=(
            -DCMAKE_C_FLAGS='-m32'
            -DCMAKE_CXX_FLAGS='-m32'
        )
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
