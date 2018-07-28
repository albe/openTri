TARGET = contest
OBJS = contest.o ../../triMemory.o ../../triConsole.o ../../triLog.o

INCDIR = 
CFLAGS = -O0 -G0 -Wall -g -D_DEBUG_LOG -D_DEBUG_MEMORY -D__PSP__
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBS = 

BUILD_PRX = 1

EXTRA_TARGETS = EBOOT.PBP

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
