CC=g++
CFLAGS=-Wall -Werror -Wextra
SOURCES=console_snake.cpp
LIBS=-lncurses

all: console_snake

console_snake:
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LIBS)

clean:
	rm console_snake

rebuild: clean all

.PHONY = all clean rebuild
