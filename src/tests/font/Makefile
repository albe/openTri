TARGET = fonttest
OBJS = fonttest.o ../../streams/streams.o ../../triVAlloc.o ../../triMemory.o ../../triRefcount.o ../../triImage.o ../../rle.o ../../triGraphics.o ../../triLog.o ../../triFont.o

PSPBIN = $(shell psp-config --psp-prefix)

INCDIR = 
CFLAGS = -O0 -G0 -Wall -g -DDEBUG -D_DEBUG -D_DEBUG_LOG -D_DEBUG_MEMORY -D__PSP__ -DTRI_SUPPORT_FT $(shell $(PSPBIN)/bin/freetype-config --cflags)
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBS = -lpspgum -lpspgu -lpsprtc -lfreetype -lm -lz $(shell $(PSPBIN)/bin/freetype-config --libs)

BUILD_PRX = 1

EXTRA_TARGETS = EBOOT.PBP

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
