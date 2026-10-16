CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, build/%.o, $(SRC))

build/mycc: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

build/%.o: src/%.c |  build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

test: build/mycc
	./tests/run_tests.sh

clean:
	rm -rf build/*.o build/mycc