CC=gcc
CFLAGS+=-std=c99 -fopenmp -lm

CHPL=chpl

mandelbrot_chapel: mandelbrot.chpl MPlot.chpl
	$(CHPL) $(CHPL_FLAGS) $^ -o $@ 

mandelbrot_c: mandelbrot.c
	$(CC) $(CFLAGS) $< -o $@

stencil9: stencil9.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f ./*.o mandelbrot_chapel mandelbrot_c stencil9
