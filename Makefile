TARGET = irongb
OBJS = src/main.o

INCDIR =
CFLAGS = -G0 -Wall -O2 -std=c11
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR = ./lib/
LIBS = -lSDL2
LIBS += -lpspaudiolib -lpspaudio
LIBS += -lpspgum -lpspgu -lm
LIBS += -lglut -lGLU -lGL
LIBS += -lpsprtc -lpspvfpu -lpspvram  -lpsphprm

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = IronGB

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
