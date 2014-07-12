#ifndef _MSC_VER
#include <stdbool.h>
#endif
#include <sched.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef __CELLOS_LV2__
//#include <malloc.h>
#endif

#ifdef _MSC_VER
#define snprintf _snprintf
#pragma pack(1)
#endif

#include "libretro.h"

#include "vdp1.h"
#include "vdp2.h"
//#include "scsp.h"
#include "peripheral.h"
#include "cdbase.h"
#include "yabause.h"
#include "yui.h"

//#include "m68kc68k.h"
#include "cs0.h"

#include "m68kcore.h"
#include "vidogl.h"
#include "vidsoft.h"

static bool failed_init;
yabauseinit_struct yinit;
static PerPad_struct *c1, *c2 = NULL;
bool FPSisOn = false;
bool firstRun = true;

uint16_t *videoBuffer = NULL;
static uint16_t frame_buffer[704*512];
int game_width;
int game_height;

static retro_video_refresh_t video_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;
static retro_environment_t environ_cb;
static retro_audio_sample_batch_t audio_batch_cb;

void retro_set_environment(retro_environment_t cb) { environ_cb = cb; }
void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb) { (void)cb; }
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }
void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }
/*
typedef struct
{
   unsigned retro;
   unsigned saturn;
} keymap;

static const keymap bindmap[] = {
   { RETRO_DEVICE_ID_JOYPAD_B, PERPAD_B },
   { RETRO_DEVICE_ID_JOYPAD_A, PERPAD_C },
   { RETRO_DEVICE_ID_JOYPAD_X, PERPAD_Y },
   { RETRO_DEVICE_ID_JOYPAD_Y, PERPAD_A },
   { RETRO_DEVICE_ID_JOYPAD_START, PERPAD_START },
   { RETRO_DEVICE_ID_JOYPAD_L, PERPAD_X },
   { RETRO_DEVICE_ID_JOYPAD_R, PERPAD_Z },
   { RETRO_DEVICE_ID_JOYPAD_UP, PERPAD_UP },
   { RETRO_DEVICE_ID_JOYPAD_DOWN, PERPAD_DOWN },
   { RETRO_DEVICE_ID_JOYPAD_LEFT, PERPAD_LEFT },
   { RETRO_DEVICE_ID_JOYPAD_RIGHT, PERPAD_RIGHT },
   { RETRO_DEVICE_ID_JOYPAD_L2, PERPAD_LEFT_TRIGGER },
   { RETRO_DEVICE_ID_JOYPAD_R2, PERPAD_RIGHT_TRIGGER }, 
};
*/
static void update_input()
{
    if (!input_poll_cb)
        return;
    
    input_poll_cb();

	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP))
		PerPadUpPressed(c1);
	else
		PerPadUpReleased(c1);
	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN))
		PerPadDownPressed(c1);
	else
		PerPadDownReleased(c1);
	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT))
		PerPadLeftPressed(c1);
	else
		PerPadLeftReleased(c1);
	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT))
		PerPadRightPressed(c1);
	else
		PerPadRightReleased(c1);
	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y))
		PerPadAPressed(c1);
	else
		PerPadAReleased(c1);
	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B))
		PerPadBPressed(c1);
	else
		PerPadBReleased(c1);
	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A))
		PerPadCPressed(c1);
	else
		PerPadCReleased(c1);
	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X))
		PerPadXPressed(c1);
	else
		PerPadXReleased(c1);
	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L))
		PerPadYPressed(c1);
	else
		PerPadYReleased(c1);
	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R))
		PerPadZPressed(c1);
	else
		PerPadZReleased(c1);
	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START))
		PerPadStartPressed(c1);
	else
		PerPadStartReleased(c1);
	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2))
		PerPadLTriggerPressed(c1);
	else
		PerPadLTriggerReleased(c1);
	if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2))
		PerPadRTriggerPressed(c1);
	else
		PerPadRTriggerReleased(c1);
}

// SNDLIBRETRO
#define SNDCORE_LIBRETRO   11
#define SAMPLERATE 44100
#define SAMPLEFRAME 735
#define BUFFER_LEN 65536

static uint32_t video_freq;
static uint32_t audio_size;

static int SNDLIBRETROInit(void);
static void SNDLIBRETRODeInit(void);
static int SNDLIBRETROReset(void);
static int SNDLIBRETROChangeVideoFormat(int vertfreq);
static void SNDLIBRETROUpdateAudio(u32 *leftchanbuffer, u32 *rightchanbuffer, u32 num_samples);
static u32 SNDLIBRETROGetAudioSpace(void);
static void SNDLIBRETROMuteAudio(void);
static void SNDLIBRETROUnMuteAudio(void);
static void SNDLIBRETROSetVolume(int volume);

static int SNDLIBRETROInit(void)
{
	//SNDLIBRETROChangeVideoFormat(60);
    return 0;
}

static void SNDLIBRETRODeInit(void)
{
}

static int SNDLIBRETROReset(void)
{
    return 0;
}

static int SNDLIBRETROChangeVideoFormat(int vertfreq)
{
	//video_freq = vertfreq;
	//audio_size = 44100 / vertfreq;
    return 0;
}

static void sdlConvert32uto16s(s32 *srcL, s32 *srcR, s16 *dst, u32 len) {
   u32 i;

   for (i = 0; i < len; i++)
   {
      // Left Channel
      if (*srcL > 0x7FFF)
         *dst = 0x7FFF;
      else if (*srcL < -0x8000)
         *dst = -0x8000;
      else
         *dst = *srcL;
      srcL++;
      dst++;

      // Right Channel
      if (*srcR > 0x7FFF)
         *dst = 0x7FFF;
      else if (*srcR < -0x8000)
         *dst = -0x8000;
      else
         *dst = *srcR;
      srcR++;
      dst++;
   } 
}

static void SNDLIBRETROUpdateAudio(u32 *leftchanbuffer, u32 *rightchanbuffer, u32 num_samples)
{
   s16 sound_buf[4096];
   sdlConvert32uto16s((s32*)leftchanbuffer, (s32*)rightchanbuffer, sound_buf, num_samples);
   audio_batch_cb(sound_buf, num_samples);

   audio_size -= num_samples;
}

static u32 SNDLIBRETROGetAudioSpace(void)
{
	return audio_size;
}

void SNDLIBRETROMuteAudio()
{
}

void SNDLIBRETROUnMuteAudio()
{
}

void SNDLIBRETROSetVolume(int volume)
{
}

SoundInterface_struct SNDLIBRETRO = {
    SNDCORE_LIBRETRO,
    "Libretro Sound Interface",
    SNDLIBRETROInit,
    SNDLIBRETRODeInit,
    SNDLIBRETROReset,
    SNDLIBRETROChangeVideoFormat,
    SNDLIBRETROUpdateAudio,
    SNDLIBRETROGetAudioSpace,
    SNDLIBRETROMuteAudio,
    SNDLIBRETROUnMuteAudio,
    SNDLIBRETROSetVolume
};

M68K_struct *M68KCoreList[] = {
    &M68KDummy,
    &M68KC68K,
    NULL
};

SH2Interface_struct *SH2CoreList[] = {
    &SH2Interpreter,
    &SH2DebugInterpreter,
    NULL
};

PerInterface_struct *PERCoreList[] = {
    &PERDummy,
	//&PERLIBRETROJoy,
	NULL
};

CDInterface *CDCoreList[] = {
    &DummyCD,
    &ISOCD,
    NULL
};

SoundInterface_struct *SNDCoreList[] = {
    &SNDDummy,
	&SNDLIBRETRO,
    NULL
};

VideoInterface_struct *VIDCoreList[] = {
    //&VIDDummy,
    &VIDSoft,
    NULL
};

#pragma mark Yabause Callbacks

void YuiErrorMsg(const char *string)
{
    printf("Yabause Error %s", string);
}

void YuiSetVideoAttribute(int type, int val)
{
    printf("Yabause called back to YuiSetVideoAttribute");
}

int YuiSetVideoMode(int width, int height, int bpp, int fullscreen)
{
    printf("Yabause called, it want to set width of %d and height of %d",width,height);
    return 0;
}

void updateCurrentResolution(void)
{
    int current_width = 320;
    int current_height = 240;

    // Test if VIDCore valid AND NOT the Dummy Interface (or at least VIDCore->id != 0). 
    // Avoid calling GetGlSize if Dummy/id=0 is selected
    if (VIDCore && VIDCore->id) 
    {
        VIDCore->GetGlSize(&current_width, &current_height);
    }
    game_width = current_width;
    game_height = current_height;
}

void YuiSwapBuffers(void) 
{
    updateCurrentResolution();
    memcpy(videoBuffer, dispbuffer, sizeof(u16) * game_width * game_height);
}

/************************************
 * libretro implementation
 ************************************/

static struct retro_system_av_info g_av_info;

void retro_get_system_info(struct retro_system_info *info)
{
    memset(info, 0, sizeof(*info));
	info->library_name = "Yabause";
	info->library_version = "v0.9.12";
	info->need_fullpath = true;
	info->valid_extensions = "bin|cue|bin|CUE";
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
    memset(info, 0, sizeof(*info));
    // Just assume NTSC for now. TODO: Verify FPS.
    info->timing.fps            = 60;
    info->timing.sample_rate    = 44100;
    info->geometry.base_width   = game_width;
    info->geometry.base_height  = game_height;
    info->geometry.max_width    = 704;
    info->geometry.max_height   = 512;
    info->geometry.aspect_ratio = 4.0 / 3.0;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
   (void)port;
   (void)device;
}

size_t retro_serialize_size(void) 
{ 
	//return STATE_SIZE;
	return 0;
}

bool retro_serialize(void *data, size_t size)
{ 
   //if (size != STATE_SIZE)
   //   return FALSE;

	ScspMuteAudio(SCSP_MUTE_SYSTEM);
	int error = YabSaveState((const char*)data);
	ScspUnMuteAudio(SCSP_MUTE_SYSTEM);
   return !error;
}

bool retro_unserialize(const void *data, size_t size)
{
   //if (size != STATE_SIZE)
   //   return FALSE;

	ScspMuteAudio(SCSP_MUTE_SYSTEM);
	int error = YabLoadState((const char*)data);
	ScspUnMuteAudio(SCSP_MUTE_SYSTEM);
   return !error;
}

void retro_cheat_reset(void)
{}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
   (void)index;
   (void)enabled;
   (void)code;
}

bool retro_load_game(const struct retro_game_info *info)
{
   const char *full_path;

   if(failed_init)
      return false;

   full_path = info->path;

   yinit.cdcoretype = CDCORE_ISO;

	yinit.cdpath = full_path;
	// Emulate BIOS
	yinit.biospath = NULL;
	
	yinit.percoretype = PERCORE_DEFAULT;
	yinit.sh2coretype = SH2CORE_INTERPRETER;
	
	yinit.vidcoretype = VIDCORE_SOFT;
	
    yinit.sndcoretype = SNDCORE_LIBRETRO;
    yinit.m68kcoretype = M68KCORE_C68K;
    yinit.carttype = 0;
    yinit.regionid = REGION_AUTODETECT;
    yinit.buppath = NULL;
    yinit.mpegpath = NULL;

    yinit.videoformattype = VIDEOFORMATTYPE_NTSC;

    yinit.frameskip = false;
    yinit.clocksync = 0;
    yinit.basetime = 0;
    yinit.usethreads = 0;

   return true;
}

bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info)
{
   (void)game_type;
   (void)info;
   (void)num_info;
   return false;
}

void retro_unload_game(void) 
{
   firstRun = true;
}

unsigned retro_get_region(void)
{
    return RETRO_REGION_NTSC;
}

unsigned retro_api_version(void)
{
    return RETRO_API_VERSION;
}

void *retro_get_memory_data(unsigned id)
{
    return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
    return 0;
}

void retro_init(void)
{
	game_width = 320;
	game_height = 240;
	
    const char *dir = NULL;
	//TODO: setup RETRO_ENVIRONMENT_GET_SAVES_DIRECTORY

	videoBuffer = (u16 *)calloc(sizeof(u16), 704 * 512);
    
	//PerPad Init
    PerPortReset();
    c1 = PerPadAdd(&PORTDATA1);
    c2 = PerPadAdd(&PORTDATA1);
	
    unsigned level = 3;
    environ_cb(RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, &level);
}

void retro_deinit(void)
{
	YabauseDeInit();
	free(videoBuffer);
}

void retro_reset(void)
{
	firstRun = true;
	YabauseResetButton();
}

void retro_run(void) 
{
	update_input();
	
   audio_size = SAMPLEFRAME;

	if(firstRun) {
		//initYabauseWithCDCore:CDCORE_DUMMY	
		YabauseInit(&yinit);
		YabauseSetDecilineMode(1);
		firstRun = false;
	}
	else {
		ScspUnMuteAudio(SCSP_MUTE_SYSTEM);
		YabauseExec();
		ScspMuteAudio(SCSP_MUTE_SYSTEM);
	}

   for (unsigned i = 0; i < game_height * game_width; i++)
   {
      uint32_t source = dispbuffer[i];

      uint16_t r = ((source & 0xF80000) >> 19);
      uint16_t g = ((source & 0x00F800) >> 6);
      uint16_t b = ((source & 0x0000F8) << 7);
      videoBuffer[i] = r | g | b;
   }
	
	video_cb(videoBuffer, game_width, game_height, game_width * 2);
}

