#!/usr/bin/env bash

: ${OS?}
: ${ARCH?}

set -eux

BASE_DIR=$(cd $(dirname $0)/.. && pwd)

FFMPEG_VERSION=8.0
FFMPEG_BUILD_TAG=v${FFMPEG_VERSION}-1

TMP_BUILD_DIR=$BASE_DIR/$(mktemp -d build.XXXXXXXX)
trap 'rm -rf $TMP_BUILD_DIR' EXIT

cd $TMP_BUILD_DIR

CMAKE_ARGS=(
    -DCMAKE_VERBOSE_MAKEFILE=ON
    -DCMAKE_BUILD_TYPE=Release
    -DBUILD_TOOLS=ON
    -DBUILD_TESTS=OFF
    -DBUILD_SHARED_LIBS=OFF
)

mkdir ffmpeg
export FFMPEG_DIR=$TMP_BUILD_DIR/ffmpeg

case $OS in
    linux)
        case $ARCH in
            x86_64)
                CMAKE_ARGS+=(
                    -DCMAKE_C_FLAGS='-static -static-libgcc -static-libstdc++'
                    -DCMAKE_CXX_FLAGS='-static -static-libgcc -static-libstdc++'
                )
                TARGET=$ARCH-linux-gnu
                ;;
            i686)
                CMAKE_ARGS+=(
                    -DCMAKE_C_FLAGS='-m32 -static -static-libgcc -static-libstdc++'
                    -DCMAKE_CXX_FLAGS='-m32 -static -static-libgcc -static-libstdc++'
                )
                TARGET=$ARCH-linux-gnu
                ;;
            arm64)
                perl -pe "s!{EXTRA_PATHS}!$FFMPEG_DIR!g" $BASE_DIR/package/toolchain-aarch64.cmake.in >toolchain.cmake
                CMAKE_ARGS+=(
                    -DCMAKE_TOOLCHAIN_FILE=$TMP_BUILD_DIR/toolchain.cmake
                )
                TARGET=$ARCH-linux-gnu
                ;;
            *)
                echo "Unsupported architecture: $ARCH"
                exit 1
                ;;
        esac
        ;;
    windows)
        TARGET=$ARCH-w64-mingw32
        perl -pe "s!{EXTRA_PATHS}!$FFMPEG_DIR!g" $BASE_DIR/package/toolchain-mingw.cmake.in | perl -pe "s!{ARCH}!$ARCH!g" >toolchain.cmake
        CMAKE_ARGS+=(
            -DCMAKE_TOOLCHAIN_FILE=$TMP_BUILD_DIR/toolchain.cmake
            -DCMAKE_C_FLAGS='-static -static-libgcc -static-libstdc++'
            -DCMAKE_CXX_FLAGS='-static -static-libgcc -static-libstdc++'
        )
        ;;
    macos)
        CMAKE_ARGS+=(
            -DCMAKE_CXX_FLAGS="-stdlib=libc++"
        )
        case $ARCH in
            x86_64)
                CMAKE_ARGS+=(
                    -DCMAKE_OSX_DEPLOYMENT_TARGET="10.9"
                    -DCMAKE_OSX_ARCHITECTURES="x86_64"
                )
                TARGET=x86_64-apple-macos10.9
                ;;
            arm64)
                CMAKE_ARGS+=(
                    -DCMAKE_OSX_DEPLOYMENT_TARGET="11.0"
                    -DCMAKE_OSX_ARCHITECTURES="arm64"
                )
                TARGET=arm64-apple-macos11
                ;;
            *)
                echo "Unsupported architecture: $OS/$ARCH"
                exit 1
                ;;
        esac
        ;;
    *)
        echo "Unsupported os: $OS"
        exit 1
        ;;
esac

curl -s -L "https://github.com/acoustid/ffmpeg-build/releases/download/$FFMPEG_BUILD_TAG/ffmpeg-$FFMPEG_VERSION-audio-$TARGET.tar.gz" | tar xz
mv ffmpeg-*/* $FFMPEG_DIR

CMAKE_ARGS+=(
    -DCMAKE_INSTALL_PREFIX=$BASE_DIR/artifacts
)

cmake "${CMAKE_ARGS[@]}" $BASE_DIR

VERSION=${GITHUB_REF##*/}

make
make install/strip
