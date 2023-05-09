#!/bin/bash

mkdir -p build_em

time emcc src/*.cc src/emulator/*.cc \
    -o build_em/gbemu.html \
    -std=c++1z \
    -O2 \
    -pthread \
    -s WASM=1 \
    -s USE_SDL=2 \
    -s USE_SDL_TTF=2 \
    -s USE_SDL_GFX=2 \
    -s USE_PTHREADS=1 \
    -s DISABLE_EXCEPTION_CATCHING=0 \
    -s PTHREAD_POOL_SIZE=1 \
    --embed-file assets/DejaVuSans.ttf \
    --embed-file roms/mario.gb \
    --embed-file roms/tetris.gb

rm /srv/www/gbemu/* -v
cp build_em/* /srv/www/gbemu -v



# 3.1.37 kaputt
# 3.1.20 kaputt
# 3.1.15 kaputt

# 3.1.14 ok
# 3.1.9 ok
# 3.0.0 ok