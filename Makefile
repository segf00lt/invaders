CC = clang
FLAGS = -g -O0 -Wall -Wpedantic -Werror -Wno-switch -Wno-comment -Wno-format-pedantic -Wno-extra-semi -D'_UNITY_BUILD_' -I'./third_party'
TARGET = invaders.c #./third_party/*.c
EXE = invaders
GAME_MODULE = invaders.so
LDFLAGS = -lraylib -lm

UNAME_S = $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	FLAGS += -Wno-format -L'/opt/homebrew/lib/' -I'/opt/homebrew/include/'
else
	FLAGS += -L'/usr/local/lib/'
endif

game_module: tags
	$(CC) $(FLAGS) -fPIC main.c -o $(EXE) $(LDFLAGS)
	$(CC) $(FLAGS) -fPIC -shared $(TARGET) -o $(GAME_MODULE)

all: tags
	$(CC) $(FLAGS) $(TARGET) -o $(EXE) $(LDFLAGS)

tags:
	ctags -w --language-force=C --c-kinds=+zfx --extras=+q --fields=+n --recurse . --exclude='*.h'
	ctags -w --language-force=C --c-kinds=+zpx --extras=+q --fields=+n -a --recurse /usr/local/include/ray*

#test: all
#	cd test/; ./unittest; cd ..;
.PHONY: all tags
