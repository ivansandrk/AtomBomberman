Atom Bomberman
============

Clone of the legendary Atomic Bomberman game http://en.wikipedia.org/wiki/Atomic_Bomberman


## Quick guide

Install dependencies (on debian):

    sudo apt-get install g++ libzzip-dev libsdl-image1.2-dev libsdl-gfx1.2-dev libsdl-mixer1.2-dev libsdl-sound1.2-dev libsdl-ttf2.0-dev


Compile:

    g++ -o main -ggdb -Wall -Wextra -O2 -D_GNU_SOURCE -I/usr/include/SDL -Iutil -lSDL -lSDL_image -lzzip -lSDL_gfx -lSDL_ttf -lSDL_mixer src/bomb.cpp src/bomber.cpp src/config.cpp src/graphics.cpp src/input.cpp src/level.cpp src/main.cpp src/sdl_renderer.cpp src/opengl_renderer.cpp src/sound.cpp util/iniparser.cpp util/SDL_rwops_zzip.c


Run:

    ./main


## Requirements
`libzzip-dev`, `libsdl-image1.2-dev`, `libsdl-gfx1.2-dev`, `libsdl-mixer1.2-dev`, `libsdl-sound1.2-dev`, `libsdl-ttf2.0-dev`
