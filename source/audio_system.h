#pragma once

#include <lauxlib.h>
#include <lua.h>

#include "spu_soundmodule.bin.h"
#include <soundlib/audioplayer.h>
#include <stdio.h>

#define MAX_CACHED_SOUNDS 8
#define MAX_SFX_BUFFER (2 * 1024 * 1024)
#define MAX_BGM_BUFFER (64 * 1024 * 1024)

void InitAudioSystem(void);

int l_PlayAudioFile(lua_State *L);
int l_SetBackgroundMusic(lua_State *L);
