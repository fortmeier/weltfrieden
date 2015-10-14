CC=gcc

NODEBUG=-DNDEBUG

CFLAGS += -g -I/usr/local/include -Iinclude -Wall -std=gnu99 $(shell pkg-config --cflags glfw3) ${NODEBUG}
LDFLAGS += -lm -L/usr/local/lib -llo -lsndfile -lsamplerate -lpthread $(shell pkg-config --libs glfw3)

ifeq ($(shell uname -s), Darwin)
		LDFLAGS += -framework OpenGL -framework Cocoa
		CFLAGS += -DMAC_OSX
else
	LDFLAGS += -lGL -ldl -lX11 -lXxf86vm -lXcursor -lXinerama -lXrandr -lXi -lepoxy -lm
endif

SOURCES=weltfrieden.c queue.c server.c shader.c
OBJECTS=$(SOURCES:.c=.o)

all: weltfrieden

clean:
	rm -f *.o *~ weltfrieden

weltfrieden: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(CFLAGS) $(LDFLAGS) -o $@

indent:
	find . \( -iname \*.[ch] -or -iname \*.frag  -or -iname \*.vert \) -exec emacs --batch -nw -q {} --eval "(progn (mark-whole-buffer) (indent-region (point-min) (point-max) nil) (save-buffer))" --kill \;
