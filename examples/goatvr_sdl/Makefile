src = $(wildcard src/*.c)
obj = $(src:.c=.o)
dep = $(obj:.o=.d)
bin = goatvr_sdl

sys := $(shell uname -s | sed 's/MINGW.*/mingw/')

ifeq ($(sys), mingw)
	lib_gl = -lopengl32 -lglu32
	goatvr_dep = libgoatvr.dll
else
	lib_gl = -lGL -lGLU
	goatvr_dep = libgoatvr.so
endif

CFLAGS = -pedantic -Wall -g -I../../include -MMD `sdl2-config --cflags`
LDFLAGS = -L. -Wl,-rpath=$(shell pwd) -lgoatvr `sdl2-config --libs` $(lib_gl) -lm

$(bin): $(obj) $(goatvr_dep)
	$(CC) -o $@ $(obj) $(LDFLAGS)

ifeq ($(sys), mingw)
libgoatvr.dll:
	cp ../../libgoatvr.dll .
else
libgoatvr.so:
	rm -f libgoatvr.so libgoatvr.so.1 libgoatvr.so.1.2
	ln -s ../../libgoatvr.so.1.2 .
	ln -s libgoatvr.so.1.2 libgoatvr.so.1
	ln -s libgoatvr.so.1 libgoatvr.so
endif

-include $(dep)

.PHONY: clean
clean:
	rm -f $(obj) $(bin)

.PHONY: cleandep
cleandep:
	rm -f $(dep)
