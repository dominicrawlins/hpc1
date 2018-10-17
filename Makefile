stencil: stencil.c
	gcc -std=c99 -Wall -Ofast $^ -pg -g -o $@


