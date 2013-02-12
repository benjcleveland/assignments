CC=gcc
CFLAGS+=-std=c99 -fopenmp -lm -lrt -D_POSIX_C_SOURCE=199309L

CHPL=chpl

mandelbrot_chapel: mandelbrot.chpl MPlot.chpl
	$(CHPL) $(CHPL_FLAGS) $^ -o $@ 

mandelbrot_c: mandelbrot.c
	$(CC) $(CFLAGS) $< -o $@

mandelbrot_dynamic_c: mandelbrot_dynamic.c
	$(CC) $(CFLAGS) $< -o $@

stencil9: stencil9.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f ./*.o mandelbrot_chapel mandelbrot_c mandelbrot_dynamic_c stencil9
