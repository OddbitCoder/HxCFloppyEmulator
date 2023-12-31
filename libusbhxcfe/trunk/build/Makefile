#CC=i386-pc-linux-gcc
CC=gcc
AR=ar

TARGET := $(shell uname)

BASEDIR=../sources
INCLUDES = -I$(BASEDIR)/ -I ../../../libhxcfe/trunk/sources -I ../sources/win32 -I ../../../libhxcadaptor/trunk/sources

DEBUG ?= 0
DEBUG_ASAN ?= 0

ifeq ($(DEBUG), 1)
	CFLAGS=-O0 $(INCLUDES) -Wall -g -DDEBUG
	LDFLAGS= -shared
else
	CFLAGS=-O3 $(INCLUDES) -Wall
	LDFLAGS= -shared -s
endif

ifeq ($(DEBUG_ASAN), 1)
	CFLAGS += -fsanitize=address -fno-omit-frame-pointer
	LDFLAGS += -static-libasan -fsanitize=address
endif

EXEC=libusbhxcfe.so

ifeq ($(TARGET), Linux)
CFLAGS += -fPIC -Wl,-Map,foo.map
LDFLAGS += -g -lc -lm  -ldl
endif

ifeq ($(TARGET), mingw32)
CC=i686-w64-mingw32-gcc
RESC=i686-w64-mingw32-windres
LDFLAGS += -static-libgcc ../sources/win32/libusbhxcfe.def
EXEC=libusbhxcfe.dll
endif

ifeq ($(TARGET), mingw64)
CC=x86_64-w64-mingw32-gcc
RESC=x86_64-w64-mingw32-windres
LDFLAGS += -static-libgcc ../sources/win32/libusbhxcfe.def
EXEC=libusbhxcfe.dll
endif

ifeq ($(TARGET), Darwin)
MACOSX_MIN_VER ?= 10.13
CFLAGS += -arch x86_64 -mmacosx-version-min=${MACOSX_MIN_VER}
LDFLAGS +=  -lc -lm -ldl -arch x86_64 -dynamiclib -current_version 2.0 -install_name @executable_path/../Frameworks/libusbhxcfe.dylib -mmacosx-version-min=${MACOSX_MIN_VER}
EXEC=libusbhxcfe.dylib
endif

ifeq ($(findstring CYGWIN,$(shell uname)),CYGWIN)
EXEC=libusbhxcfe.dll
endif

all: $(EXEC)

libusbhxcfe.dll: win32_libusbhxcfe_res.o usb_hxcfloppyemulator.o variablebitrate.o ftdi.o ../../../build/libhxcadaptor.a ../../../build/libhxcfe.dll
	$(CC) -o $@    $^ $(LDFLAGS)
	cp $@ ../../../build

libusbhxcfe.dylib: usb_hxcfloppyemulator.o variablebitrate.o ftdi.o
	$(AR) r $@    $^
	cp $@ ../../../build

libusbhxcfe.a: usb_hxcfloppyemulator.o variablebitrate.o ftdi.o
	$(AR) r $@    $^
	cp $@ ../../../build

libusbhxcfe.so: usb_hxcfloppyemulator.o variablebitrate.o ftdi.o
	$(CC) -o $@    $^ $(LDFLAGS)
	cp $@ ../../../build

win32_libusbhxcfe_res.o: $(BASEDIR)/win32/libusbhxcfe.rc
	$(RESC) $< $@

usb_hxcfloppyemulator.o: $(BASEDIR)/usb_hxcfloppyemulator.c
	$(CC) -o $@ -c $< $(CFLAGS)

variablebitrate.o: $(BASEDIR)/variablebitrate.c
	$(CC) -o $@ -c $< $(CFLAGS)

ftdi.o: $(BASEDIR)/ftdi.c
	$(CC) -o $@ -c $< $(CFLAGS)

ftdi_d2xxmode.o : ftdi.c ftdi.h ftd2xx.h WinTypes.h
	$(CC) -o $@ -c $< $(CFLAGS)

ftdi_libftdimode.o : ftdi.c ftdi.h ftd2xx.h WinTypes.h
	$(CC) -o $@ -c $< $(CFLAGS) -DFTDILIB

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)
	rm -rf *.dll
	rm -rf *.dylib

.PHONY: clean mrproper
