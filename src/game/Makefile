TARGET = game
OBJS = gameMain.o gameBase.o ../streams/streams.o ../triTexman.o ../triCamera.o ../triVAlloc.o ../triMemory.o ../triRefcount.o ../triImage.o ../rle.o ../triGraphics.o ../tri3d.o ../triLog.o ../triModel.o ../triInput.o ../triVMath_vfpu.o ../triAt3.o ../triWav.o ../triAudioLib.o ../triNet.o ../triError.o

INCDIR = 
CFLAGS = -O2 -G0 -Wall -g -DTRI_SUPPORT_SAVE_TRIM -DDEBUG -D_DEBUG -D_DEBUG_LOG -D_DEBUG_MEMORY -D__PSP__
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBS = -lpspgum -lpspgu -lpsprtc -lm -lz -lpspwlan -lpspaudio -lpspaudiocodec -lcurl

BUILD_PRX = 1

EXTRA_TARGETS = EBOOT.PBP

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
