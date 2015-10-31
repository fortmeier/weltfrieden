CC=gcc

NODEBUG=-DNDEBUG

CFLAGS += -g -Wall -std=gnu99 ${NODEBUG}


ifeq ($(shell uname -s), Darwin)
		LDFLAGS += -L/usr/local/lib -framework OpenGL -framework Cocoa
		CFLAGS += $(shell pkg-config --cflags glfw3 freetype2) -I/usr/local/include -DMAC_OSX
else
#  ifeq ($(shell uname -m), armv71) # e.g.: rasperry pi 2
	CFLAGS += -DEGL_RPI2 # -I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/include/interface/vmcs_host/linux
	# LDFLAGS += -L/opt/vc/lib -lGLESv2 -lEGL -lm -lbcm_host -ldl
	LDFLAGS += -llo # $(shell pkg-config --libs glfw3) $(shell pkg-config --libs freetype2)

#  else
#	LDFLAGS += -lGL -ldl -lX11 -lXxf86vm -lXcursor -lXinerama -lXrandr -lXi -lepoxy -lm
#  endif
endif

OBJS=font.o vgft.o graphics.o
LIB=libvgfont.a

INCLUDES+=-I$(SDKSTAGE)/usr/include/freetype2 -I$(SDKSTAGE)/usr/include -I$(SDKSTAGE)/usr/include/arm-linux-gnueabi


SOURCES=weltfrieden.c queue.c server.c layer.c shader.c text.c layers.c

# lib/vgfont/vgft.c lib/vgfont/font.c 
BIN=weltfrieden.bin
OBJS=$(SOURCES:.c=.o)

include lib/Makefile.include

# all: weltfrieden

# clean:
# 	rm -f *.o *~ weltfrieden

# machinearch:
# 	@echo $(shell uname -m)

# weltfrieden: $(OBJECTS) Makefile machinearch
# 	$(CC) $(OBJECTS) $(CFLAGS) $(LDFLAGS) -o $@


test:
	valgrind --suppressions=valgrind-opengl-10-11-manual.supp --suppressions=valgrind-opengl-10-11.supp --leak-check=full --gen-suppressions=all -v -d ./weltfrieden -w 256 -h 256 --cache

indent:
	find . \( -iname \*.[ch] -or -iname \*.frag  -or -iname \*.vert \) -exec emacs --batch -nw -q {} --eval "(progn (mark-whole-buffer) (indent-region (point-min) (point-max) nil) (save-buffer))" --kill \;
