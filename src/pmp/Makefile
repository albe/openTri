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
PSPDIR=$(shell psp-config --psp-prefix)
include $(PSPSDK)/lib/build.mak

install: $(TARGET_LIB)
	@$(CP) -v libpmp.a $(PSPDIR)/lib
	@$(CP) -v pmp.h $(PSPDIR)/include
	@echo Done.
