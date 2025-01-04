CC = clang++ -std=c++11
FLAGS = -g -O0 -Wall -Wpedantic -Werror -Wno-switch -Wno-comment -Wno-format-pedantic -Wno-narrowing -Wno-c99-extensions -D'_UNITY_BUILD_'
TARGET = invaders.cpp
LDFLAGS = -lraylib -lm

UNAME_S = $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	FLAGS += -Wno-format -L'/opt/homebrew/lib/' -I'/opt/homebrew/include/'
#else
#	LDFLAGS += -L'/usr/local/lib/'
endif

all:
	ctags -w *.cpp *.h
	$(CC) $(FLAGS) $(TARGET) -o invaders $(LDFLAGS)

#test: all
#	cd test/; ./unittest; cd ..;
.PHONY: all
