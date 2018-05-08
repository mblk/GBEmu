#!/bin/bash

mkdir -p build_em

time emcc src/*.cc src/emulator/*.cc \
    -o build_em/gbemu_asmjs.html \
    -std=c++1z \
    -s USE_SDL=2 \
    --embed-file tetris.gb \
    -O2 \
    -s WASM=0 \
    -s USE_PTHREADS=0 \
    -s DISABLE_EXCEPTION_CATCHING=0

#time emcc src/*.cc src/emulator/*.cc \
#    -o build_em/gbemu_wasm.html \
#    -std=c++1z \
#    -s USE_SDL=2 \
#    --embed-file tetris.gb \
#    -O2 \
#    -s WASM=1 \
#    -s USE_PTHREADS=0 \
#    -s DISABLE_EXCEPTION_CATCHING=0


#--embed-file mario.gb \

rm /var/www/html/gbemu/* -v
cp build_em/* /var/www/html/gbemu -v
