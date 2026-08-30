CC = clang
CFLAGS = -Wall -Wextra -std=c11 -g
SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, build/%.o, $(SRC))

build/mycc: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

build/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

test: build/mycc
	./tests/run_tests.sh

clean:
	rm -rf build/*.o build/mycc