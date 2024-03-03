CC = gcc
CFLAGS = -Wall -Wextra
INCLUDES = ./include

bmp-demo: src/bmp-demo.c
	$(CC) -o $@ $^ -I$(INCLUDES) $(CFLAGS)

.PHONY: clean

clean:
	rm bmp-demo

