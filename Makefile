CC=gcc

CFLAGS += -g -I/usr/local/include -Wall -O3 -std=gnu99
LDFLAGS += -lm -L/usr/local/lib -llo -lsndfile -lsamplerate -lpthread

ifeq ($(shell uname -s), Darwin)
		LDFLAGS += -framework GLUT -framework OpenGL -framework Cocoa
		CFLAGS += -DMAC_OSX
else
		LDFLAGS += -lglut -lGLU
endif

SOURCES=weltfrieden.c server.c
OBJECTS=$(SOURCES:.c=.o)


#weltfrieden: CFLAGS +=
#weltfrieden:

all: weltfrieden

clean:
	rm -f *.o *~ weltfrieden

weltfrieden: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(CFLAGS) $(LDFLAGS) -o $@
