#include "character.h"
#include "lauxlib.h"
#include "lua.h"
#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>

char say_name[64];
char say_text[1024];
double say_start;
Color say_name_color;

TextBuffer TextBoxTextBuffer = {0};

Character *chars[MAX_CHARACTERS];

Character *InitCharacter(const char *name, Color color) {
  Character *c = calloc(1, sizeof(Character));
  if (!c)
    return NULL;
  c->name = name;
  c->color = color;
  return c;
}

int l_InitCharacter(lua_State *L) {
  int id = luaL_checkint(L, 1);
  const char *name = luaL_checkstring(L, 2);
  Color color = {luaL_checkint(L, 3), luaL_checkint(L, 4), luaL_checkint(L, 5),
                 luaL_checkint(L, 6)};

  chars[id] = InitCharacter(name, color);
  return 0;
}

int l_CharacterSay(lua_State *L) {
  int id = luaL_checkint(L, 1);
  const char *text = luaL_checkstring(L, 2);

  snprintf(say_text, sizeof(say_text), "%s", text);
  snprintf(say_name, sizeof(say_name), "%s", chars[id]->name);
  say_name_color = chars[id]->color;

  say_start = GetTime();
  return 0;
}
