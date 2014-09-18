src = $(wildcard src/*.c)
obj = $(src:.c=.o)
bin = test

vrlib_root = ..
vrlib = $(vrlib_root)/libgoatvr.a

CFLAGS = -pedantic -Wall -g `pkg-config --cflags sdl2` -I$(vrlib_root)/src
LDFLAGS = $(libgl) `pkg-config --libs sdl2` $(vrlib)

ifeq ($(shell uname -s), Darwin)
	libgl = -framework OpenGL -lGLEW
else
	libgl = -lGL -lGLU -lGLEW
endif

$(bin): $(obj) $(vrlib)
	$(CC) -o $@ $(obj) $(LDFLAGS)

.PHONY: $(vrlib)
$(vrlib):
	$(MAKE) -C $(vrlib_root)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)
