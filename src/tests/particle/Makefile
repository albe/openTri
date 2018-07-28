TARGET = particletest
OBJS = particletest.o ../../streams/streams.o ../../triParticle.o ../../triTexman.o ../../triCamera.o ../../triVAlloc.o \
       ../../triMemory.o ../../triRefcount.o ../../triImage.o ../../rle.o ../../triGraphics.o ../../tri3d.o \
       ../../triLog.o ../../triInput.o ../../triVMath_vfpu.o ../../triTimer.o ../../triFont.o

INCDIR = 
CFLAGS = -O3 -G0 -Wall -g -DDEBUG -D_DEBUG -D_DEBUG_LOG -D_DEBUG_MEMORY
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBS = -lpspgum -lpspgu -lpsprtc -lm -lz

BUILD_PRX = 1

EXTRA_TARGETS = EBOOT.PBP

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
