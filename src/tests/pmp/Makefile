TARGET = pmptest
OBJS = pmptest.o ../../triLog.o

BUILD_PRX = 1

INCDIR = 
CFLAGS = -O0 -G0 -Wall -g -D__PSP__
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LIBS = -lpspctrl -lpsppower -lpspaudiocodec -lpspaudio -lpspmpeg -lpspgum -lpspgu -lpsprtc

ifeq ($(LIBPMP),1)
LIBS += -lpmp
else
OBJS += ../../pmp/audiodecoder.o ../../pmp/avc.o ../../pmp/mem64.o ../../pmp/pmp.o ../../pmp/pmp_decode.o ../../pmp/pmp_file.o ../../pmp/pmp_play.o ../../pmp/pmp_read.o ../../pmp/mpegbase.o
endif

EXTRA_TARGETS = EBOOT.PBP

all: clean

libpmp:
	@make LIBPMP=1 all

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
