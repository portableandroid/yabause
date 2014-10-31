DEBUG = 0
DYNAREC = 0
HAVE_THREADS = 1
HAVE_GRIFFIN = 1

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

# system platform
system_platform = unix
ifeq ($(shell uname -a),)
EXE_EXT = .exe
   system_platform = win
else ifneq ($(findstring Darwin,$(shell uname -a)),)
   system_platform = osx
	arch = intel
ifeq ($(shell uname -p),powerpc)
	arch = ppc
endif
else ifneq ($(findstring MINGW,$(shell uname -a)),)
   system_platform = win
endif

YABAUSE_DIR := ../yabause
SOURCE_DIR := $(YABAUSE_DIR)/src

TARGET_NAME = yabause
CC_AS ?= $(CC)

ifeq ($(platform), unix)
   TARGET := $(TARGET_NAME)_libretro.so
   fpic := -fPIC
   SHARED := -shared -Wl,--no-undefined -Wl,--version-script=link.T
   ENDIANNESS_DEFINES := -DLSB_FIRST
	#DYNAREC = 2
	LDFLAGS += -lpthread
	HAVE_PTHREADS = 1
else ifeq ($(platform), osx)
   TARGET := $(TARGET_NAME)_libretro.dylib
   fpic := -fPIC -mmacosx-version-min=10.6
   SHARED := -dynamiclib
   ENDIANNESS_DEFINES := -DLSB_FIRST
   #DYNAREC = 2
	LDFLAGS += -lpthread
	HAVE_PTHREADS = 1
else ifeq ($(platform), ios)
   TARGET := $(TARGET_NAME)_libretro_ios.dylib
   fpic := -fPIC
   SHARED := -dynamiclib
   ENDIANNESS_DEFINES := -DLSB_FIRST
   CC = clang -arch armv7 -isysroot $(IOSSDK)
   CC_AS =  perl ./libretro/gas-preprocessor.pl $(CC)
   CXX = clang++ -arch armv7 -isysroot $(IOSSDK)
else ifeq ($(platform), qnx)
   TARGET := $(TARGET_NAME)_libretro_qnx.so
   fpic := -fPIC
   SHARED := -shared -Wl,--version-script=libretro/link.T
	CC = qcc -Vgcc_ntoarmv7le
   CC_AS = $(CC)
	CXX = QCC -Vgcc_ntoarmv7le
	AR = qcc -Vgcc_ntoarmv7le
	ENDIANNESS_DEFINES += -D__BLACKBERRY_QNX__ -marm -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=softfp -DLSB_FIRST
else ifeq ($(platform), ps3)
   TARGET := $(TARGET_NAME)_libretro_ps3.a
   CC = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-gcc.exe
   CXX = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-g++.exe
   AR = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-ar.exe
   ENDIANNESS_DEFINES = -DWORDS_BIGENDIAN -DMSB_FIRST -D__ppc__
   STATIC_LINKING = 1
else ifeq ($(platform), sncps3)
   TARGET := $(TARGET_NAME)_libretro_ps3.a
   CC = $(CELL_SDK)/host-win32/sn/bin/ps3ppusnc.exe
   CXX = $(CELL_SDK)/host-win32/sn/bin/ps3ppusnc.exe
   AR = $(CELL_SDK)/host-win32/sn/bin/ps3snarl.exe
   ENDIANNESS_DEFINES = -DWORDS_BIGENDIAN -DMSB_FIRST -D__ppc__
   STATIC_LINKING = 1
else ifeq ($(platform), psl1ght)
   TARGET := $(TARGET_NAME)_libretro_psl1ght.a
   CC = $(PS3DEV)/ppu/bin/ppu-gcc$(EXE_EXT)
   CC = $(PS3DEV)/ppu/bin/ppu-g++$(EXE_EXT)
   AR = $(PS3DEV)/ppu/bin/ppu-ar$(EXE_EXT)
   ENDIANNESS_DEFINES = -DWORDS_BIGENDIAN -DMSB_FIRST -D__ppc__
	STATIC_LINKING = 1
else ifeq ($(platform), psp1)
	TARGET := $(TARGET_NAME)_libretro_psp1.a
	CC = psp-gcc$(EXE_EXT)
	CC_AS = $(CC)
	CXX = psp-g++$(EXE_EXT)
	AR = psp-ar$(EXE_EXT)
	ENDIANNESS_DEFINES += -DPSP -G0 -DLSB_FIRST
	STATIC_LINKING = 1
else ifeq ($(platform), ngc)
   TARGET := $(TARGET_NAME)_libretro_ngc.a
   CC = $(DEVKITPPC)/bin/powerpc-eabi-gcc$(EXE_EXT)
   CXX = $(DEVKITPPC)/bin/powerpc-eabi-g++$(EXE_EXT)
   AR = $(DEVKITPPC)/bin/powerpc-eabi-ar$(EXE_EXT)
   ENDIANNESS_DEFINES = -DWORDS_BIGENDIAN -DMSB_FIRST -DGEKKO -DHW_DOL -mrvl -mcpu=750 -meabi -mhard-float -D__ppc__
   STATIC_LINKING = 1
else ifeq ($(platform), wii)
   TARGET := $(TARGET_NAME)_libretro_wii.a
   CC = $(DEVKITPPC)/bin/powerpc-eabi-gcc$(EXE_EXT)
   CXX = $(DEVKITPPC)/bin/powerpc-eabi-g++$(EXE_EXT)
   AR = $(DEVKITPPC)/bin/powerpc-eabi-ar$(EXE_EXT)
   ENDIANNESS_DEFINES = -DWORDS_BIGENDIAN -DMSB_FIRST -DGEKKO -DHW_RVL -mrvl -mcpu=750 -meabi -mhard-float -D__ppc__
   STATIC_LINKING = 1
else
   TARGET := $(TARGET_NAME)_libretro.dll
   CC = gcc
   CXX = g++
   SHARED := -shared -Wl,--no-undefined -Wl,--version-script=link.T
   LDFLAGS += -static-libgcc -static-libstdc++ -lwinmm
   ENDIANNESS_DEFINES := -DLSB_FIRST
	FLAGS += -DHAVE_WFOPEN
	#DYNAREC = 2
endif

ifneq ($(HAVE_GRIFFIN), 1)
ifeq ($(HAVE_PTHREADS),1)
	HAVE_THREADS = 1
	THREAD_FILES := thr-unix.c
else
	THREAD_FILES := $(SOURCE_DIR)/thr-dummy.c
endif
endif

SOURCES_C := $(SOURCE_DIR)/osdcore.c \
	$(SOURCE_DIR)/bios.c \
	$(SOURCE_DIR)/c68k/c68k.c \
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
	$(SOURCE_DIR)/memory.c \
	$(SOURCE_DIR)/movie.c \
	$(SOURCE_DIR)/netlink.c \
	$(SOURCE_DIR)/peripheral.c \
	$(SOURCE_DIR)/profile.c \
	$(SOURCE_DIR)/scsp2.c \
	$(SOURCE_DIR)/scu.c \
	$(SOURCE_DIR)/sh2core.c \
	$(SOURCE_DIR)/sh2idle.c \
	$(SOURCE_DIR)/sh2int.c \
	$(SOURCE_DIR)/smpc.c \

ifeq ($(HAVE_GRIFFIN), 1)
	SOURCES_C += yabause_griffin.c
else
	SOURCES_C +=  $(SOURCE_DIR)/snddummy.c \
	$(THREAD_FILES) \
	$(SOURCE_DIR)/vdp1.c \
	$(SOURCE_DIR)/vdp2.c \
	$(SOURCE_DIR)/vidshared.c \
	$(SOURCE_DIR)/vidsoft.c \
	$(SOURCE_DIR)/yabause.c
endif

ifeq ($(HAVE_GL),1)
SOURCES_C += \
	$(SOURCE_DIR)/ygl.c \
	$(SOURCE_DIR)/vidogl.c \
	$(SOURCE_DIR)/yglshader.c
endif

ifneq ($(HAVE_GRIFFIN), 1)
LIBRETRO_SOURCES := libretro.c
endif

C68KEXEC_SOURCE := $(SOURCE_DIR)/c68k/c68kexec.c
C68KEXEC_OBJECT := $(C68KEXEC_SOURCE:.c=.o)

ifneq ($(DYNAREC),0)
FLAGS += -DSH2_DYNAREC
SOURCES += $(SOURCE_DIR)/sh2_dynarec/sh2_dynarec.c
ifeq ($(DYNAREC),2)
OBJECTS += $(SOURCE_DIR)/sh2_dynarec/linkage_x64.o
endif
endif

ifeq ($(HAVE_THREADS),1)
FLAGS += -DHAVE_THREADS=1
endif

SOURCES += $(LIBRETRO_SOURCES) $(YABAUSE_SOURCES)
OBJECTS += $(SOURCES:.c=.o) $(SOURCES_C:.c=.o) $(C68KEXEC_OBJECT)

all: $(TARGET)

ifeq ($(DEBUG), 0)
   FLAGS += -O3
else
   FLAGS += -O0 -g
endif

LDFLAGS += $(fpic) $(SHARED)
FLAGS += $(fpic)
FLAGS += -I. -I../yabause -I../yabause/src

WARNINGS :=
FLAGS += -D__LIBRETRO__ $(ENDIANNESS_DEFINES) -DSIZEOF_DOUBLE=8 $(WARNINGS) \
			-DUSE_SCSP2=1 -DNO_CLI -DHAVE_SYS_TIME_H -DHAVE_GETTIMEOFDAY -DVERSION=\"0.9.12\" -DHAVE_STRCASECMP

CXXFLAGS += $(FLAGS)

CFLAGS += $(FLAGS)
ifeq ($(platform),qnx)
CFLAGS += -Wc,-std=c99
else
CFLAGS += -std=gnu99
endif

$(TARGET): $(OBJECTS)
ifeq ($(STATIC_LINKING), 1)
	$(AR) rcs $@ $(OBJECTS)
else
	$(CXX) -o $@ $^ $(LDFLAGS)
endif

%.o: %.S
	$(CC_AS) $(CFLAGS) -c $^ -o $@

%.o: %.s
	$(CC_AS) $(CFLAGS) -c $^ -o $@

%.o: %.cpp
	$(CXX) -c -o $@ $< $(CXXFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)
	
$(C68KEXEC_OBJECT): $(C68KEXEC_SOURCE)
	$(CC) -c -o $@ $< $(CFLAGS) -O0

clean:
	rm -f $(TARGET) $(OBJECTS)

.PHONY: clean
