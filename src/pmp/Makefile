### 
## libpmp
###

TARGET_LIB = libpmp.a
OBJS   = audiodecoder.o avc.o mem64.o mpegbase.o pmp.o pmp_decode.o pmp_file.o pmp_play.o pmp_read.o

INCDIR = 
CFLAGS = -G0 -O2 -Wall -g #-mno-explicit-relocs

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LDFLAGS =
LIBS = -lpspctrl -lpsppower -lpspaudiocodec -lpspmpeg -lpspgu

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

all: libpmp

libpmp:
	rm -f *.o libpmp.a
	make TARGET_LIB=libpmp.a OBJS=$(OBJS) libpmp.a
	psp-ar cru libpmp.a $(OBJS)
	psp-ranlib libpmp.a

install:
	@cp -v libpmp.a `psp-config --psp-prefix`/lib
	@cp -v pmp.h `psp-config --psp-prefix`/include
	@echo "Done."

clean:
	rm -f *.o *.a
