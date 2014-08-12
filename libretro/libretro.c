#ifndef _MSC_VER
#include <stdbool.h>
#endif
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _MSC_VER
#define snprintf _snprintf
#pragma pack(1)
#endif

#include <sys/stat.h>

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

yabauseinit_struct yinit;

uint16_t *vid_buf = NULL;
int game_width;
int game_height;

static bool hle_bios_force = false;
static bool frameskip_enable = false;

struct retro_perf_callback perf_cb;
retro_get_cpu_features_t perf_get_cpu_features_cb = NULL;

retro_log_printf_t log_cb;
static retro_video_refresh_t video_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;
static retro_environment_t environ_cb;
static retro_audio_sample_batch_t audio_batch_cb;

void retro_set_environment(retro_environment_t cb)
{
   static const struct retro_variable vars[] = {
      { "yabause_frameskip", "Frameskip; disabled|enabled" },
      { "yabause_force_hle_bios", "Force HLE BIOS (restart); disabled|enabled" },
      { NULL, NULL },
   };

   static const struct retro_controller_description peripherals[] = {
       { "Saturn Pad", RETRO_DEVICE_JOYPAD },
       { "Saturn 3D Pad", RETRO_DEVICE_ANALOG },
       { "None", RETRO_DEVICE_NONE },
   };
   
   static const struct retro_controller_info ports[] = {
      { peripherals, 3 },
      { peripherals, 3 },
      { peripherals, 3 },
      { peripherals, 3 },
      { peripherals, 3 },
      { peripherals, 3 },
      { peripherals, 3 },
      { peripherals, 3 },
      { 0 },
   };
   
   environ_cb = cb;

   cb(RETRO_ENVIRONMENT_SET_VARIABLES, (void*)vars);
   environ_cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports);
}
void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb) { (void)cb; }
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }
void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }

// PERLIBRETRO
#define PERCORE_LIBRETRO 2
//Libretro API limited to 8 players? Saturn supports up to 12
#define MAX_PLAYERS 8
static void *controller[MAX_PLAYERS] = {0};
static int pad_type[MAX_PLAYERS] = {1,1,1,1,1,1,1,1};

int PERLIBRETROInit(void)
{
    PortData_struct* portdata = 0;
    int i = 0;
    
    PerPortReset();
    
    for(i = 0; i < MAX_PLAYERS; i++) {
        //Ports can handle 6 peripherals, fill port 1 first.
        if(i < 6)
            portdata = &PORTDATA1;
        else
            portdata = &PORTDATA2;
        switch(pad_type[i]){
            case RETRO_DEVICE_NONE:
                //Use add PerAddPeripherial to add not present ID?
                controller[i] = 0;
                break;
            case RETRO_DEVICE_ANALOG:
                controller[i] = (void*)Per3DPadAdd(portdata);
                break;
            case RETRO_DEVICE_JOYPAD:
            default:
                controller[i] = (void*)PerPadAdd(portdata);
                break;
        }
    }
    return 0;
}

static int PERLIBRETROHandleEvents(void)
{
   if (!input_poll_cb)
      return 1;

   input_poll_cb();

   int i = 0;
   PerAnalog_struct* analog = 0;
   PerPad_struct* digital = 0;
   
   for(i = 0; i < MAX_PLAYERS; i++) {
      analog = (PerAnalog_struct*)controller[i];
      digital = (PerPad_struct*)controller[i];
      
      switch(pad_type[i]){
         case RETRO_DEVICE_ANALOG:
         {
         int analog_left_x = input_state_cb(i, RETRO_DEVICE_ANALOG, 
            RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_X);

         int analog_left_y = input_state_cb(i, RETRO_DEVICE_ANALOG,
            RETRO_DEVICE_INDEX_ANALOG_LEFT, RETRO_DEVICE_ID_ANALOG_Y);

            //u32 l_right = analog_left_x > 0 ?  analog_left_x : 0;
            //u32 l_left  = analog_left_x < 0 ? -analog_left_x : 0;
            //u32 l_down  = analog_left_y > 0 ?  analog_left_y : 0;
            //u32 l_up    = analog_left_y < 0 ? -analog_left_y : 0;

            PerAxis1Value(analog, (analog_left_x + 0x8000) >> 8);
            PerAxis2Value(analog, (analog_left_y + 0x8000) >> 8);
            //PerAxis3Value((PerAnalog_struct*)controller[i], l_down >> 8);
            //PerAxis4Value((PerAnalog_struct*)controller[i], l_up >> 8);
         }
         case RETRO_DEVICE_JOYPAD:
         {
         if (input_state_cb(i, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP))
            PerPadUpPressed(digital);
         else
            PerPadUpReleased(digital);
         if (input_state_cb(i, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN))
            PerPadDownPressed(digital);
         else
            PerPadDownReleased(digital);
         if (input_state_cb(i, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT))
            PerPadLeftPressed(digital);
         else
            PerPadLeftReleased(digital);
         if (input_state_cb(i, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT))
            PerPadRightPressed(digital);
         else
            PerPadRightReleased(digital);
         if (input_state_cb(i, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y))
            PerPadAPressed(digital);
         else
            PerPadAReleased(digital);
         if (input_state_cb(i, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B))
            PerPadBPressed(digital);
         else
            PerPadBReleased(digital);
         if (input_state_cb(i, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A))
            PerPadCPressed(digital);
         else
            PerPadCReleased(digital);
         if (input_state_cb(i, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X))
            PerPadXPressed(digital);
         else
            PerPadXReleased(digital);
         if (input_state_cb(i, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L))
            PerPadYPressed(digital);
         else
            PerPadYReleased(digital);
         if (input_state_cb(i, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R))
            PerPadZPressed(digital);
         else
            PerPadZReleased(digital);
         if (input_state_cb(i, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START))
            PerPadStartPressed(digital);
         else
            PerPadStartReleased(digital);
         if (input_state_cb(i, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L2))
            PerPadLTriggerPressed(digital);
         else
            PerPadLTriggerReleased(digital);
         if (input_state_cb(i, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R2))
            PerPadRTriggerPressed(digital);
         else
            PerPadRTriggerReleased(digital);
         }
         break;

         default:
         break;
      }
   }
    return 0;
}

void PERLIBRETRODeInit(void) {
   /* Nothing */
}

void PERLIBRETRONothing(void) {
   /* Nothing */
}

u32 PERLIBRETROScan(u32 flags) {
   /* Nothing */
   return 0;
}

void PERLIBRETROKeyName(u32 key, char *name, int size) {
   /* Nothing */
}

PerInterface_struct PERLIBRETROJoy = {
    PERCORE_LIBRETRO,
    "Libretro Input Interface",
    PERLIBRETROInit,
    PERLIBRETRODeInit,
    PERLIBRETROHandleEvents,
    PERLIBRETROScan,
    0,
    PERLIBRETRONothing,
    PERLIBRETROKeyName
};

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
    &PERLIBRETROJoy,
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
   if (log_cb)
      log_cb(RETRO_LOG_ERROR, "Yabause: %s\n", string);
}

void YuiSetVideoAttribute(int type, int val)
{
   if (log_cb)
      log_cb(RETRO_LOG_INFO, "Yabause called back to YuSetVideoAttribute.\n");
}

int YuiSetVideoMode(int width, int height, int bpp, int fullscreen)
{
   if (log_cb)
      log_cb(RETRO_LOG_INFO, "Yabause called, it wants to set width of %d and height of %d.\n", width, height);
    return 0;
}

void updateCurrentResolution(void)
{
    int current_width = 320;
    int current_height = 240;

    // Test if VIDCore valid AND NOT the Dummy Interface (or at least VIDCore->id != 0). 
    // Avoid calling GetGlSize if Dummy/id=0 is selected
    if (VIDCore && VIDCore->id) 
       VIDCore->GetGlSize(&current_width, &current_height);

    game_width = current_width;
    game_height = current_height;
}

void YuiSwapBuffers(void) 
{
    updateCurrentResolution();
}

/************************************
 * libretro implementation
 ************************************/

static struct retro_system_av_info g_av_info;

void retro_get_system_info(struct retro_system_info *info)
{
    memset(info, 0, sizeof(*info));
	info->library_name = "Yabause";
	info->library_version = "v0.9.13";
	info->need_fullpath = true;
	info->valid_extensions = "bin|cue|iso";
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
   pad_type[port] = device;
   if(PERCore) PERCore->Init();
   //update_peripherals();
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

static char full_path[256];
static char bios_path[256];

static void check_variables(void)
{
   struct retro_variable var;
   var.key = "yabause_frameskip";
   var.value = NULL;

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (strcmp(var.value, "disabled") == 0 && frameskip_enable)
      {
         DisableAutoFrameSkip();
         frameskip_enable = false;
      }
      else if (strcmp(var.value, "enabled") == 0 && !frameskip_enable)
      {
         EnableAutoFrameSkip();
         frameskip_enable = true;
      }
   }

   var.key = "yabause_force_hle_bios";
   var.value = NULL;

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value)
   {
      if (strcmp(var.value, "disabled") == 0 && frameskip_enable)
         hle_bios_force = false;
      else if (strcmp(var.value, "enabled") == 0 && !frameskip_enable)
         hle_bios_force = true;
   }
}

static int does_file_exist(const char *filename)
{
   struct stat st;
   int result = stat(filename, &st);
   return result == 0;
}

void retro_init(void)
{
   struct retro_log_callback log;

   if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
      log_cb = log.log;
   else
      log_cb = NULL;

   if (environ_cb(RETRO_ENVIRONMENT_GET_PERF_INTERFACE, &perf_cb))
      perf_get_cpu_features_cb = perf_cb.get_cpu_features;
   else
      perf_get_cpu_features_cb = NULL;

	game_width = 320;
	game_height = 240;
	
   const char *dir = NULL;
   environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir);

   if (dir)
   {
#ifdef _WIN32
      char slash = '\\';
#else
      char slash = '/';
#endif
      snprintf(bios_path, sizeof(bios_path), "%s%c%s", dir, slash, "saturn_bios.bin");
   }

	vid_buf = (u16 *)calloc(sizeof(u16), 704 * 512);
    
   if(PERCore) PERCore->Init();
    //update_peripherals();
	
    // Performance level for interpreter CPU core is 16
    unsigned level = 16;
    environ_cb(RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, &level);
}

bool retro_load_game(const struct retro_game_info *info)
{
   check_variables();

   snprintf(full_path, sizeof(full_path), "%s", info->path);

   yinit.cdcoretype = CDCORE_ISO;

   yinit.cdpath = full_path;
   // Emulate BIOS
   yinit.biospath = (bios_path[0] != '\0' && does_file_exist(bios_path) && !hle_bios_force) ? bios_path : NULL;

   yinit.percoretype = PERCORE_LIBRETRO;
#ifdef SH2_DYNAREC
   yinit.sh2coretype = 2;
#else
   yinit.sh2coretype = SH2CORE_INTERPRETER;
#endif

   yinit.vidcoretype = VIDCORE_SOFT;

   yinit.sndcoretype = SNDCORE_LIBRETRO;
   yinit.m68kcoretype = M68KCORE_C68K;
   yinit.carttype = 0;
   yinit.regionid = REGION_AUTODETECT;
   yinit.buppath = NULL;
   yinit.mpegpath = NULL;

   yinit.videoformattype = VIDEOFORMATTYPE_NTSC;

   yinit.frameskip = frameskip_enable;
   yinit.clocksync = 0;
   yinit.basetime = 0;
#ifdef HAVE_THREADS
   yinit.usethreads = 1;
#else
   yinit.usethreads = 0;
#endif

   YabauseInit(&yinit);
   YabauseSetDecilineMode(1);

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
	YabauseDeInit();
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


void retro_deinit(void)
{
   if (vid_buf)
      free(vid_buf);
}

void retro_reset(void)
{
	YabauseResetButton();
   YabauseInit(&yinit);
   YabauseSetDecilineMode(1);
}

void retro_run(void) 
{
   bool updated = false;

   if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated)
      check_variables();
   
   if(PERCore) PERCore->HandleEvents();
   //update_input();
	
   audio_size = SAMPLEFRAME;

   YabauseExec();

   for (unsigned i = 0; i < game_height * game_width; i++)
   {
      uint32_t source = dispbuffer[i];

      uint16_t r = ((source & 0xF80000) >> 19);
      uint16_t g = ((source & 0x00F800) >> 6);
      uint16_t b = ((source & 0x0000F8) << 7);
      vid_buf[i] = r | g | b;
   }
	
	video_cb(vid_buf, game_width, game_height, game_width * 2);
}

#ifdef ANDROID
#include <wchar.h>

size_t mbstowcs(wchar_t *pwcs, const char *s, size_t n)
{
   if (pwcs == NULL)
      return strlen(s);
   return mbsrtowcs(pwcs, &s, n, NULL);
}

size_t wcstombs(char *s, const wchar_t *pwcs, size_t n)
{
   return wcsrtombs(s, &pwcs, n, NULL);
}

#endif
