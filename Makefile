stencil: stencil.c
	icc -std=c99 -Wall -Ofast  $^  -g -o $@


