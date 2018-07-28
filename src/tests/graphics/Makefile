TARGET = gfxtest
OBJS = gfxtest.o ../../streams/streams.o ../../triVAlloc.o ../../triMemory.o ../../triRefcount.o ../../triImage.o ../../rle.o ../../triGraphics.o ../../triLog.o

BUILD_PRX = 1

INCDIR = 
CFLAGS = -O2 -G0 -Wall -g -DDEBUG -D_DEBUG -D_DEBUG_LOG -D_DEBUG_MEMORY -D__PSP__ -DTRI_SUPPORT_PNG
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBS = -lpspgum -lpspgu -lpsprtc -lpng -lm -lz

BUILD_PRX = 1

EXTRA_TARGETS = EBOOT.PBP

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
