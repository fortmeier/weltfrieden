CC=gcc


CFLAGS += -g -I/usr/local/include -Iinclude -Wall -O3 -std=gnu99 $(shell pkg-config --cflags glfw3)
LDFLAGS += -lm -L/usr/local/lib -llo -lsndfile -lsamplerate -lpthread $(shell pkg-config --libs glfw3) -lGLEW

ifeq ($(shell uname -s), Darwin)
		LDFLAGS += -framework OpenGL -framework Cocoa
		CFLAGS += -DMAC_OSX
else
		LDFLAGS += -lglut -lGLU -lGLEW
endif

SOURCES=weltfrieden.c server.c shader.c
OBJECTS=$(SOURCES:.c=.o)


#weltfrieden: CFLAGS +=
#weltfrieden:

all: weltfrieden

clean:
	rm -f *.o *~ weltfrieden

weltfrieden: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(CFLAGS) $(LDFLAGS) -o $@
