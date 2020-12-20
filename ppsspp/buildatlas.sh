#!/bin/bash
mkdir -p ext/native/tools/build/ && pwd
cd ext/native/tools/build/ && cmake .. && export MAKEFLAGS=-j$(nproc) && make && cd ../../../../ && pwd
./ext/native/tools/build/atlastool atlasscript.txt ui 8888
cp ui_atlas.zim ui_atlas.meta assets
rm -f ui_atlas.zim ui_atlas.meta ui_atlas.cpp ui_atlas.h ui_atlas.zim.png
