#pragma once

#include "lua.h"
#include <raylib.h>
#include <stdlib.h>

#define MAX_CHARACTERS 16

extern char say_name[64];
extern char say_text[1024];
extern double say_start;
extern Color say_name_color;

typedef enum CHARACTER_POSITION { LEFT, CENTER, RIGHT } char_pos;

typedef struct {
  const char *name;
  Color color;

  Texture sprite;
  int sprite_scale;
  bool has_sprite;
  char_pos character_position;
} Character;

typedef struct text_buffer {
  const char *name;
  const char *text;
  int show_percent;
} TextBuffer;

extern TextBuffer TextBoxTextBuffer;

extern Character *chars[MAX_CHARACTERS];

Character *InitCharacter(const char *name, Color color);
int l_InitCharacter(lua_State *L);
int l_CharacterSay(lua_State *L);
