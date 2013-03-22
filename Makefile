CC     = gcc
CPP    = g++
LIBS   = -lSDL -lGL -lGLU -lSDL_image
FLAGS  = $(LIBS) -ggdb -Wall

.cpp: $@.cpp
	$(CPP) $@.cpp -o $@ $(FLAGS)

.c: $@.c
	$(CC) -o $@ $@.c $(FLAGS)

#%.cpp: $*.cpp
#	$(CPP) $*.cpp -o $* $(FLAGS)

#ani_parse: ani_parse.c
#	$(CC) ani_parse.c -o ani_parse -lm

MAIN_SOURCES = config.c opengl_renderer.c graphics.c sdl_renderer.c iniparser.c dictionary.c SDL_rwops_zzip.c bomber.c input.c level.c bomb.c main.c
MAIN_HEADERS = config.h opengl_renderer.h graphics.h sdl_renderer.h iniparser.h dictionary.h SDL_rwops_zzip.h bomber.h input.h level.h bomb.h
main: $(MAIN_SOURCES) $(MAIN_HEADERS)
	$(CC) $(MAIN_SOURCES) -o main -Wextra -Wall -I/usr/include/SDL -lSDL -lGL -lSDL_image -lzzip -D_GNU_SOURCE -ggdb -lSDL_gfx -lSDL_ttf -O2
# config.c opengl_renderer.c graphics.c sdl_renderer.c bomber.c input.c level.c main.c config.h opengl_renderer.h graphics.h sdl_renderer.h bomber.h input.h level.h

