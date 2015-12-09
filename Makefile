CC=gcc

NODEBUG=-DNDEBUG

CFLAGS += -g -I/usr/local/include -Wall -std=gnu99 $(shell pkg-config --cflags glfw3) ${NODEBUG}
LDFLAGS += -lm -L/usr/local/lib -llo -lpthread $(shell pkg-config --libs glfw3)

ifeq ($(shell uname -s), Darwin)
		LDFLAGS += -framework OpenGL -framework Cocoa
		CFLAGS += -DMAC_OSX
else
	LDFLAGS += -lGL -ldl -lX11 -lXxf86vm -lXcursor -lXinerama -lXrandr -lXi -lepoxy -lm
endif

SOURCES=weltfrieden.c queue.c server.c layer.c shader.c layers.c thpool.c jobqueue.c
OBJECTS=$(SOURCES:.c=.o)

all: weltfrieden

clean:
	rm -f *.o *~ weltfrieden

weltfrieden: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(CFLAGS) $(LDFLAGS) -o $@

test:
	valgrind --suppressions=valgrind-opengl-10-11-manual.supp --suppressions=valgrind-opengl-10-11.supp --leak-check=full --gen-suppressions=all -v -d ./weltfrieden -w 256 -h 256 --cache
