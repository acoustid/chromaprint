#!/usr/bin/env bash

: ${OS?}
: ${ARCH?}
: ${TAG:=}
: ${BRANCH?}

set -eux

BASE_DIR=$(cd $(dirname $0)/.. && pwd)

TMP_BUILD_DIR=$BASE_DIR/$(mktemp -d build.XXXXXXXX)
trap 'rm -rf $TMP_BUILD_DIR' EXIT

cd $TMP_BUILD_DIR

FFMPEG_BUILD_VERSION=v4.4.1-1

case $OS-$ARCH in
  macos-arm64)
    TARGET=$ARCH-apple-macos11
    ;;
  macos-x86_64)
    TARGET=$ARCH-apple-macos10.8
    ;;
  windows-*)
    TARGET=$ARCH-w64-mingw32
    ;;
  linux-*)
    TARGET=$ARCH-linux-gnu
    ;;
esac

curl -s -L "https://github.com/acoustid/ffmpeg-build/releases/download/$FFMPEG_BUILD_VERSION/ffmpeg-$FFMPEG_BUILD_VERSION-audio-$TARGET.tar.gz" | tar xz
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
        -DHAVE_AV_PACKET_UNREF=1
        -DHAVE_AV_FRAME_ALLOC=1
        -DHAVE_AV_FRAME_FREE=1
    )
    STRIP=$ARCH-w64-mingw32-strip
    ;;
macos)
    CMAKE_ARGS+=(
        -DCMAKE_CXX_FLAGS="-stdlib=libc++ -target=$TARGET"
    )
    ;;
linux)
    case $ARCH in
    i686)
        CMAKE_ARGS+=(
            -DCMAKE_C_FLAGS='-m32 -static -static-libgcc -static-libstdc++'
            -DCMAKE_CXX_FLAGS='-m32 -static -static-libgcc -static-libstdc++'
        )
        ;;
    x86_64)
        CMAKE_ARGS+=(
            -DCMAKE_C_FLAGS='-static -static-libgcc -static-libstdc++'
            -DCMAKE_CXX_FLAGS='-static -static-libgcc -static-libstdc++'
        )
        ;;
    arm*)
        perl -pe "s!{EXTRA_PATHS}!$FFMPEG_DIR!g" $BASE_DIR/package/toolchain-armhf.cmake.in | perl -pe "s!{ARCH}!$ARCH!g" >toolchain.cmake
        CMAKE_ARGS+=(
            -DCMAKE_TOOLCHAIN_FILE=$TMP_BUILD_DIR/toolchain.cmake
            -DCMAKE_C_FLAGS='-static -static-libgcc -static-libstdc++'
            -DCMAKE_CXX_FLAGS='-static -static-libgcc -static-libstdc++'
        )
        STRIP=arm-linux-gnueabihf-strip
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

cmake "${CMAKE_ARGS[@]}" $BASE_DIR

make VERBOSE=1
make install VERBOSE=1

$STRIP $BASE_DIR/chromaprint-$OS-$ARCH/bin/fpcalc*

case $TAG in
v*)
    VERSION=$(echo $TAG | sed 's/^v//')
    ;;
*)
    VERSION=$BRANCH-$(date +%Y%m%d%H%M)
    ;;
esac

FPCALC_DIR=chromaprint-fpcalc-$VERSION-$OS-$ARCH
rm -rf $FPCALC_DIR
mkdir $FPCALC_DIR
cp $BASE_DIR/chromaprint-$OS-$ARCH/bin/fpcalc* $FPCALC_DIR

case $OS in
windows)
    zip -r $BASE_DIR/$FPCALC_DIR.zip $FPCALC_DIR
    ;;
*)
    tar -zcvf $BASE_DIR/$FPCALC_DIR.tar.gz $FPCALC_DIR
    ;;
esac
