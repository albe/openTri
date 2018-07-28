TARGET = at3test
OBJS = at3test.o ../../../triLog.o ../../../triAt3.o ../../../triMemory.o

INCDIR = 
CFLAGS = -O0 -G0 -Wall -g -D_DEBUG_LOG -D_DEBUG_MEMORY
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBS = -lpspaudio -lpspaudiocodec

BUILD_PRX = 1

EXTRA_TARGETS = EBOOT.PBP

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
