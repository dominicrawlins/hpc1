stencil: stencil.c
	icc -std=c99 -Wall -O3 -ftree-vectorize -fopt-info-vec-missed $^ -pg -g -o $@


