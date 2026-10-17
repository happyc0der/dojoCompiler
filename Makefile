CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
SRC := $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, build/%.o, $(SRC))
TEST = $(filter-out build/main.o, $(OBJ))
.PHONY: clean test
build/mycc: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

build/%.o: src/%.c |  build
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

test: build/test_lexer 
	./$<

build/test_lexer: tests/test_lexer.c $(TEST) | build
	$(CC) $(CFLAGS) -o $@ $^

build:
	mkdir -p build

clean:
	rm -rf build/

-include  $(OBJ:.o=.d)