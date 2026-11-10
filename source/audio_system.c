#include "audio_system.h"
#include "lauxlib.h"
#include "lua.h"
#include "soundlib/audioplayer.h"
#include "soundlib/spu_soundlib.h"
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

static short *bgm_buffer = NULL;
static int bgm_cap = 0;

typedef struct {
  short *buffer; // выделяется ОДИН раз при первом использовании
  int buf_size;  // реальный размер выделенного буфера
  int voice;     // какой голос играет этот слот, -1 = свободен
} SfxSlot;

static SfxSlot slots[MAX_CACHED_SOUNDS];
static int next_slot = 0;

void InitAudioSystem(void) {
  u32 spu;
  sysSpuImage spu_image;
  sysSpuInitialize(1, 5);
  sysSpuRawCreate(&spu, NULL);
  sysSpuImageImport(&spu_image, spu_soundmodule_bin, 0);
  sysSpuRawImageLoad(spu, &spu_image);
  SND_Init(spu);

  for (int i = 0; i < MAX_CACHED_SOUNDS; i++) {
    slots[i].voice = -1;
  }
  SND_Pause(0);
}

static SfxSlot *pick_slot(int *next_slot) {
  SfxSlot *best = NULL;

  for (int i = 0; i < MAX_CACHED_SOUNDS; i++) {
    SfxSlot *s = &slots[i];
    if (s->voice < 0) {
      return s;
    } else if (SND_StatusVoice(s->voice) == SND_UNUSED) {
      return s;
    }
  }

  best = &slots[*next_slot];
  *next_slot = (*next_slot + 1) % MAX_CACHED_SOUNDS;
  if (best->voice >= 0) {
    SND_StopVoice(best->voice);
  }
  best->voice = -1;
  return best;
}

static int find_free_voice(void) {
  for (int i = 2; i < MAX_SND_VOICES; i++) {
    if (SND_StatusVoice(i) == SND_UNUSED) {
      return i;
    }
  }
  return -1;
}

int l_PlayAudioFile(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  int volume = (int)luaL_optinteger(L, 2, 255);

  // first finding free voice (voice == -1),
  // else pick next_slot by round and stop their voice
  SfxSlot *s =
      pick_slot(&next_slot); // round-robin: next_slot = (next_slot+1) % MAX
  if (!s->buffer) {
    s->buffer = memalign(32, MAX_SFX_BUFFER);
    if (!s->buffer)
      return luaL_error(L, "out of memory");
  }
  if (s->voice >= 0)
    SND_StopVoice(s->voice);
  s->voice = -1;

  // reading file to temp buffer
  FILE *f = fopen(path, "r");
  if (!f) {
    lua_pushnil(L);
    return 1;
  }

  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  rewind(f);

  unsigned char *fdata = malloc(fsize); // temp data for file
  fread(fdata, 1, fsize, f);
  fclose(f);

  int decoded = MAX_SFX_BUFFER;
  int freq, stereo;
  int ok = DecodeAudio(fdata, fsize, s->buffer, &decoded, &freq, &stereo);

  free(fdata);
  if (ok != 0 || decoded == 0) {
    return luaL_error(L, "decode failed: ok=%d dec=%d freq=%d st=%d", ok,
                      decoded, freq, stereo);
  }

  int voice = find_free_voice();
  if (voice < 0) {
    lua_pushnil(L);
    return 1;
  }

  decoded = (decoded + 31) & ~31;

  int snd_ok =
      SND_SetVoice(voice, stereo ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT, freq,
                   0, s->buffer, decoded, volume, volume, NULL);
  if (snd_ok != SND_OK) {
    return luaL_error(L, "setvoice: ret=%d voice=%d size=%d", snd_ok, voice,
                      decoded);
  }
  int vstat = SND_StatusVoice(voice);
  if (vstat != SND_WORKING) {
    return luaL_error(L, "voice stat=%d (unused=0 working=1)", vstat);
  }
  s->voice = voice;
  lua_pushinteger(L, voice); // handle for stop/pause
  return 1;
}

int l_SetBackgroundMusic(lua_State *L) {
  const char *filepath = luaL_checkstring(L, 1);
  int volume = (int)luaL_optinteger(L, 2, 255);

  SND_StopVoice(1);

  FILE *file = fopen(filepath, "r");
  if (!file) {
    lua_pushnil(L);
    return 1;
  }

  fseek(file, 0, SEEK_END);
  long fsize = ftell(file);
  rewind(file);
  if (fsize <= 0) {
    fclose(file);
    lua_pushnil(L);
    return 1;
  }

  unsigned char *fdata = malloc(fsize);
  if (!fdata) {
    fclose(file);
    lua_pushnil(L);
    return 1;
  }
  fread(fdata, 1, fsize, file);
  fclose(file);

  free(bgm_buffer);

  bgm_cap = (int)fsize * 24;
  if (bgm_cap > MAX_BGM_BUFFER) {
    bgm_cap = MAX_BGM_BUFFER;
  }

  bgm_buffer = memalign(32, bgm_cap);
  if (!bgm_buffer) {
    free(fdata);
    lua_pushnil(L);
    return 1;
  }

  int decoded = bgm_cap;
  int freq, stereo;
  int ok = DecodeAudio(fdata, fsize, bgm_buffer, &decoded, &freq, &stereo);

  free(fdata);
  if (ok != 0) {
    lua_pushnil(L);
    return 1;
  }

  if (decoded >= bgm_cap || decoded == 0) {
    lua_pushstring(L, "bgm too long for buffer");
    lua_error(L);
  }

  decoded = (decoded + 31) & ~31;

  SND_SetInfiniteVoice(1, stereo ? VOICE_STEREO_16BIT : VOICE_MONO_16BIT, freq,
                       0, bgm_buffer, decoded, volume, volume);
  return 0;
}

int l_StopBackgroundMusic(lua_State *L) {
  SND_StopVoice(1);
  return 0;
}

int l_PauseBackgroundMusic(lua_State *L) {
  SND_PauseVoice(1, 1);
  return 0;
}

int l_UnpauseBackgroundMusic(lua_State *L) {
  SND_PauseVoice(1, 0);
  return 0;
}

int l_StopSound(lua_State *L) {
  int voice = luaL_checkint(L, 1);
  SND_StopVoice(voice);
}
