
DEBUG = 0

ifeq ($(platform),)
platform = unix
ifeq ($(shell uname -a),)
   platform = win
else ifneq ($(findstring Darwin,$(shell uname -a)),)
   platform = osx
else ifneq ($(findstring MINGW,$(shell uname -a)),)
   platform = win
endif
endif

TARGET_NAME = yabause

ifeq ($(platform), unix)
   TARGET := $(TARGET_NAME)_libretro.so
   fpic := -fPIC
   SHARED := -shared -Wl,--no-undefined -Wl,--version-script=link.T
   ENDIANNESS_DEFINES := -DLSB_FIRST
   FLAGS += -DHAVE_MKDIR
else ifeq ($(platform), osx)
   TARGET := $(TARGET_NAME)_libretro.dylib
   fpic := -fPIC -mmacosx-version-min=10.6
   SHARED := -dynamiclib
   ENDIANNESS_DEFINES := -DLSB_FIRST
   FLAGS += -DHAVE_MKDIR
else
   TARGET := $(TARGET_NAME)_libretro.dll
   CC = gcc
   CXX = g++
   SHARED := -shared -Wl,--no-undefined -Wl,--version-script=link.T
   LDFLAGS += -static-libgcc -static-libstdc++ -lwinmm
   ENDIANNESS_DEFINES := -DLSB_FIRST
   FLAGS += -DHAVE__MKDIR
endif


YABAUSE_DIR := ../yabause
SOURCE_DIR := $(YABAUSE_DIR)/src

SOURCES_C := $(SOURCE_DIR)/osdcore.c \
	$(SOURCE_DIR)/bios.c \
	$(SOURCE_DIR)/c68k/c68k.c \
	$(SOURCE_DIR)/c68k/gen68k.c \
	$(SOURCE_DIR)/cdbase.c \
	$(SOURCE_DIR)/cheat.c \
	$(SOURCE_DIR)/coffelf.c \
	$(SOURCE_DIR)/cs0.c \
	$(SOURCE_DIR)/cs1.c \
	$(SOURCE_DIR)/cs2.c \
	$(SOURCE_DIR)/debug.c \
	$(SOURCE_DIR)/error.c \
	$(SOURCE_DIR)/japmodem.c \
	$(SOURCE_DIR)/m68kc68k.c \
	$(SOURCE_DIR)/m68kcore.c \
	$(SOURCE_DIR)/m68kd.c \
	$(SOURCE_DIR)/memory.c \
	$(SOURCE_DIR)/movie.c \
	$(SOURCE_DIR)/netlink.c \
	$(SOURCE_DIR)/peripheral.c \
	$(SOURCE_DIR)/profile.c \
	$(SOURCE_DIR)/scsp2.c \
	$(SOURCE_DIR)/scu.c \
	$(SOURCE_DIR)/sh2core.c \
	$(SOURCE_DIR)/sh2d.c \
	$(SOURCE_DIR)/sh2idle.c \
	$(SOURCE_DIR)/sh2int.c \
	$(SOURCE_DIR)/sh2trace.c \
	$(SOURCE_DIR)/smpc.c \
	$(SOURCE_DIR)/snddummy.c \
	$(SOURCE_DIR)/sndwav.c \
	$(SOURCE_DIR)/thr-dummy.c \
	$(SOURCE_DIR)/titan/titan.c \
	$(SOURCE_DIR)/vdp1.c \
	$(SOURCE_DIR)/vdp2.c \
	$(SOURCE_DIR)/vdp2debug.c \
	$(SOURCE_DIR)/vidogl.c \
	$(SOURCE_DIR)/vidshared.c \
	$(SOURCE_DIR)/vidsoft.c \
	$(SOURCE_DIR)/yabause.c \
	$(SOURCE_DIR)/ygl.c \
	$(SOURCE_DIR)/yglshader.c

LIBRETRO_SOURCES := libretro.c

C68KEXEC_SOURCE := $(SOURCE_DIR)/c68k/c68kexec.c
C68KEXEC_OBJECT := $(C68KEXEC_SOURCE:.c=.o)

SOURCES := $(LIBRETRO_SOURCES) $(YABAUSE_SOURCES)
OBJECTS := $(SOURCES:.c=.o) $(SOURCES_C:.c=.o) $(C68KEXEC_OBJECT)

all: $(TARGET)

ifeq ($(DEBUG), 0)
   FLAGS += -O3
else
   FLAGS += -O0 -g
endif

LDFLAGS += $(fpic) -lz $(SHARED)
FLAGS += $(fpic)
FLAGS += -I. -I../yabause -I../yabause/src

WARNINGS := -Wall \
	-Wno-narrowing \
	-Wno-sign-compare \
	-Wno-unused-variable \
	-Wno-unused-function \
	-Wno-uninitialized \
	-Wno-unused-result \
	-Wno-strict-aliasing \
	-Wno-overflow

FLAGS += -D__LIBRETRO__ $(ENDIANNESS_DEFINES) -DSIZEOF_DOUBLE=8 $(WARNINGS) \
			-DUSE_SCSP2=1 -DNO_CLI -DHAVE_SYS_TIME_H -DHAVE_GETTIMEOFDAY -DVERSION=\"0.9.12\" -DHAVE_STRCASECMP

CXXFLAGS += $(FLAGS)
CFLAGS += $(FLAGS) -std=gnu99

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)
	
$(C68KEXEC_OBJECT): $(C68KEXEC_SOURCE)
	$(CC) -c -o $@ $< $(CFLAGS) -O0

clean:
	rm -f $(TARGET) $(OBJECTS)

.PHONY: clean
