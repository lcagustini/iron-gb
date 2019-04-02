all:
	@mkdir -p bin
	gcc src/main.c -o bin/main -lSDL2 -O3

debug:
	@mkdir -p bin
	gcc src/main.c -o bin/main -lSDL2 -Og -ggdb -g3 -fno-omit-frame-pointer

run: all
	./bin/main tetris.gb

clean:
	rm -rf bin
