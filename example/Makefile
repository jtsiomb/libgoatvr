src = $(wildcard src/*.c)
obj = $(src:.c=.o)
bin = test

vrlib_root = ..
vrlib = $(vrlib_root)/libgoatvr.so

warn = -Wall

CFLAGS = -pedantic $(warn) -g `pkg-config --cflags sdl2` -I$(vrlib_root)/src
LDFLAGS = $(libgl) `pkg-config --libs sdl2` -L$(vrlib_root) -lgoatvr -Wl,-rpath -Wl,$(vrlib_root)/

ifeq ($(shell uname -s), Darwin)
	libgl = -framework OpenGL -lGLEW
	warn += -Wno-deprecated-declarations
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
