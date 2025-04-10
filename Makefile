CC = clang
FLAGS = -g -O0 -Wall -Wpedantic -Werror -Wno-switch -Wno-comment -Wno-format-pedantic -Wno-extra-semi -D'_UNITY_BUILD_' -I'./third_party'
TARGET = invaders.c #./third_party/*.c
EXE = invaders
LDFLAGS = -lraylib -lm
METAPROGRAM_EXE = metaprogram

UNAME_S = $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	RAYLIB_HEADER_PATTERN = /opt/homebrew/include/ray* /opt/homebrew/include/rlgl.h
	GAME_MODULE = invaders.dylib
	SHARED = -dynamiclib
	FLAGS += -L'/opt/homebrew/lib/' -I'/opt/homebrew/include/' -Wno-format
else
	RAYLIB_HEADER_PATTERN = /usr/local/include/ray* /usr/local/include/rlgl.h
	GAME_MODULE = invaders.so
	SHARED = -shared
	FLAGS += -L'/usr/local/lib/' -I'/usr/local/include/'
endif

first: game_module

metaprogram: tags
	$(CC) $(FLAGS) -fPIC metaprogram.c -o $(METAPROGRAM_EXE) $(LDFLAGS)

run_metaprogram:
	./metaprogram 2>&1 | tee metaprogram.log

game_module: run_metaprogram tags
	$(CC) $(FLAGS) -fPIC main.c -o $(EXE) $(LDFLAGS)
	$(CC) $(FLAGS) -fPIC $(SHARED) $(TARGET) -o $(GAME_MODULE) $(LDFLAGS)

all: tags
	$(CC) $(FLAGS) $(TARGET) -o $(EXE) $(LDFLAGS)

tags:
	ctags -w --language-force=C --c-kinds=+zfx --extras=+q --fields=+n --recurse . --exclude='*.h'
	ctags -w --language-force=C --c-kinds=+zpx --extras=+q --fields=+n -a --recurse $(RAYLIB_HEADER_PATTERN)

#test: all
#	cd test/; ./unittest; cd ..;
.PHONY: all tags game_module
