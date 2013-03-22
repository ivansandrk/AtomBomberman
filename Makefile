CC      = gcc
LDFLAGS = -lSDL -lGL -lSDL_image -lzzip -lSDL_gfx -lSDL_ttf -ggdb
CFLAGS  = -ggdb -Wall -Wextra -O2 -D_GNU_SOURCE -I/usr/include/SDL -Iutil

BIN     = main
SOURCES = config.c opengl_renderer.c graphics.c sdl_renderer.c bomber.c input.c level.c bomb.c main.c
HEADERS = config.h opengl_renderer.h graphics.h sdl_renderer.h bomber.h input.h level.h bomb.h

# UTIL doesn't change
UTIL_SOURCES = iniparser.c dictionary.c SDL_rwops_zzip.c
UTIL_HEADERS = iniparser.h dictionary.h SDL_rwops_zzip.h utlist.h
#gcc util/dictionary.c -o obj/dictionary.o -c -O2
#gcc util/iniparser.c -o obj/iniparser.o -c -O2
#gcc util/SDL_rwops_zzip.c -o obj/SDL_rwops_zzip.o -c -O2 -I/usr/include/SDL

OBJECTS = $(SOURCES:.c=.o)

tt:
	echo $(OBJECTS)

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) -o $(BIN) $(OBJS) $(LDFLAGS)

clean:
	rrm -f $(OBJS) $(BIN)


.cpp: $@.cpp
	$(CPP) $@.cpp -o $@ $(FLAGS)

.c: $@.c
	$(CC) -o $@ $@.c $(FLAGS)
#%.cpp: $*.cpp
#	$(CPP) $*.cpp -o $* $(FLAGS)
#ani_parse: ani_parse.c
#	$(CC) ani_parse.c -o ani_parse -lm


main: src/$(SOURCES) src/$(HEADERS)
	$(CC) src/$(SOURCES) -o $(BIN) $(CFLAGS) $(LDFLAGS)

