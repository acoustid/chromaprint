#!/usr/bin/env bash

set -eux

cd $(dirname $0)/../..
git subtree pull --squash --prefix src/3rdparty/kissfft https://github.com/mborgerding/kissfft febd4caeed32e33ad8b2e0bb5ea77542c40f18ec
