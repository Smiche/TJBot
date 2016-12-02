#!/bin/sh
export CURRENT_DIR=`pwd`
export LD_PRELOAD="$CURRENT_DIR/libtjbot.so:$HOME/Documents/et-sdl-sound.so"
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.

cd /usr/local/games/enemy-territory/
vblank_mode=0 ./et.x86 $*

unset LD_PRELOAD
