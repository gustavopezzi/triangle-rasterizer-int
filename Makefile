build:
	gcc -Wall ./src/*.c `sdl2-config --libs --cflags` -lm -o rasterizer

run:
	./rasterizer

clean:
	rm rasterizer