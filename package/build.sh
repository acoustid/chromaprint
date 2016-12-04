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

curl -s -L "https://code.oxygene.sk/acoustid/ffmpeg-build/builds/artifacts/master/download?job=$OS+$ARCH" > artifacts.zip
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
OSX_CXX=/usr/local/bin/g++-5
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
    x86_64|armhf)
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
