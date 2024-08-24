#!/bin/bash
rm -rf build/ .cache/
cmake -DCMAKE_C_COMPILER=clang \
-DCMAKE_CXX_COMPILER=clang++ \
-DSYS_GL_HEADERS=yes \
-DCMAKE_BUILD_TYPE=Release \
-S . -B build/
