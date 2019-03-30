multiple_lines:
	@mkdir -p bin
	gcc src/main.c -o bin/main -lSDL2 -O3 -DDEBUG -g

single_line:
	@mkdir -p bin
	gcc src/main.c -o bin/main -lSDL2 -O3 -DSINGLE_OUTPUT -DDEBUG -g

no_lines:
	@mkdir -p bin
	gcc src/main.c -o bin/main -lSDL2 -O3

run: no_lines
	./bin/main

run_multiple: multiple_lines
	./bin/main

run_single: single_line
	./bin/main

clean:
	rm -rf bin
