multiple_lines:
	@mkdir -p bin
	gcc src/main.c -o bin/main

single_line:
	@mkdir -p bin
	gcc src/main.c -o bin/main -DSINGLE_OUTPUT

run: multiple_lines
	./bin/main

run_single: single_line
	./bin/main

clean:
	rm -rf bin
