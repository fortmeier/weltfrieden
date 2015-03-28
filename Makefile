CC=gcc

CFLAGS += -g -I/usr/local/include -Wall -O3 -std=gnu99
LDFLAGS += -lm -L/usr/local/lib -llo -lsndfile -lsamplerate -lpthread

SOURCES=weltfrieden.c server.c 
OBJECTS=$(SOURCES:.c=.o)

weltfrieden: CFLAGS += 
weltfrieden: LDFLAGS += -lglut -lGLU

all: weltfrieden

clean:
	rm -f *.o *~ weltfrieden 

weltfrieden: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(CFLAGS) $(LDFLAGS) -o $@
