multiple_lines:
	@mkdir -p bin
	gcc src/main.c -o bin/main -lSDL2 -O2

single_line:
	@mkdir -p bin
	gcc src/main.c -o bin/main -lSDL2 -O2 -DSINGLE_OUTPUT

run: multiple_lines
	./bin/main

run_single: single_line
	./bin/main

clean:
	rm -rf bin
