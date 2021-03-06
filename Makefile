CC      = gcc
C++     = g++
LDFLAGS = -ggdb -Wall -Wextra -O2 -lSDL -lGL -lSDL_image -lzzip -lSDL_gfx -lSDL_ttf -lSDL_mixer
CFLAGS  = -std=c++11 -ggdb -Wall -Wextra -O2 -D_GNU_SOURCE -I/usr/include/SDL -Iutil/

BIN     = main
SRCDIR  = src
OBJDIR  = obj
UTILDIR = util
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
UTIL_SOURCES = $(addprefix util/, iniparser.cpp SDL_rwops_zzip.c)

OBJECTS      = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
#OBJECTS      = $(addprefix $(OBJDIR)/, $(notdir $(SOURCES:.cpp=.o)))
UTIL_OBJECTS = $(addprefix $(OBJDIR)/, iniparser.o SDL_rwops_zzip.o)


all: $(BIN)

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(C++) $(CFLAGS) -c $< -o $@

obj/iniparser.o:
	$(C++) $(CFLAGS) -c util/iniparser.cpp -o obj/iniparser.o
obj/SDL_rwops_zzip.o:
	$(CC) $(CFLAGS) -c util/SDL_rwops_zzip.c -o obj/SDL_rwops_zzip.o

$(BIN): $(OBJECTS) $(UTIL_OBJECTS)
	$(C++) -o $@ $(OBJECTS) $(UTIL_OBJECTS) $(LDFLAGS)

clean:
	rm -f $(OBJECTS) $(BIN)

CFLAGS += -MMD
-include $(OBJECTS:.o=.d)
