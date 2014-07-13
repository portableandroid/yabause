LOCAL_PATH := $(call my-dir)
DYNAREC := 0

include $(CLEAR_VARS)

APP_DIR := ../../src

LOCAL_MODULE    := retro

ifeq ($(TARGET_ARCH),arm)
LOCAL_CFLAGS += -DANDROID_ARM
LOCAL_ARM_MODE := arm
endif

ifeq ($(TARGET_ARCH),x86)
LOCAL_CFLAGS +=  -DANDROID_X86
endif

ifeq ($(TARGET_ARCH),mips)
LOCAL_CFLAGS += -DANDROID_MIPS -D__mips__ -D__MIPSEL__
endif

YABAUSE_DIR := ../../yabause
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
	$(SOURCE_DIR)/vidshared.c \
	$(SOURCE_DIR)/vidsoft.c \
	$(SOURCE_DIR)/yabause.c

ifeq ($(HAVE_GL),1)
SOURCES_C += \
	$(SOURCE_DIR)/ygl.c \
	$(SOURCE_DIR)/vidogl.c \
	$(SOURCE_DIR)/yglshader.c
endif

LIBRETRO_SOURCES := ../libretro.c

C68KEXEC_SOURCE := $(SOURCE_DIR)/c68k/c68kexec.c

ifneq ($(DYNAREC),0)
FLAGS += -DSH2_DYNAREC
SOURCES += $(SOURCE_DIR)/sh2_dynarec/sh2_dynarec.c
ifeq ($(DYNAREC),3)
SOURCES += $(SOURCE_DIR)/sh2_dynarec/linkage_arm.s
endif
ifeq ($(DYNAREC),2)
SOURCES += $(SOURCE_DIR)/sh2_dynarec/linkage_x64.s
endif

endif

LOCAL_SRC_FILES    += $(SOURCES) $(SOURCES_C) $(LIBRETRO_SOURCES) $(C68KEXEC_SOURCE)
LOCAL_CFLAGS += -O2 -std=gnu99 -DINLINE=inline -LSB_FIRST -D__LIBRETRO__ -DFRONTEND_SUPPORTS_RGB565 -DVERSION=\"0.9.12\" -DUSE_SCSP2=1 -DNO_CLI -DHAVE_GETTIMEOFDAY -I$(SOURCE_DIR) -DHAVE_STRCASECMP

include $(BUILD_SHARED_LIBRARY)
