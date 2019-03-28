all:
	@mkdir -p bin
	gcc src/main.c -o bin/main

run: all
	./bin/main

clean:
	rm -rf bin
