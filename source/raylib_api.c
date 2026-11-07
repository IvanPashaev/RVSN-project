#include <lauxlib.h>
#include <lua.h>
#include <raylib.h>
#include <string.h>

#include "audio_system.h"
#include "character.h"

char *TextWordWrap(const char *text, float maxWidth, Font font, float fontSize,
                   float spacing) {
  if (text == NULL)
    return NULL;

  int length = strlen(text);
  // allocate memory for new string with capacity for '\n'
  char *result = (char *)malloc(length * 2 + 1);
  if (result == NULL)
    return NULL;

  int rIdx = 0;          // index in result string
  int lineStartIdx = 0;  // index of start of string in result
  int lastSpaceIdx = -1; // index of space in result

  int i = 0;
  while (text[i] != '\0') {
    int bytesProcessed = 0;
    // safely read one UTF-8 symbol (codepoint)
    int codepoint = GetCodepointNext(&text[i], &bytesProcessed);

    // remembering last space
    if (codepoint == ' ') {
      lastSpaceIdx = rIdx;
    }

    // copy symbol (all their bytes) in result string
    for (int b = 0; b < bytesProcessed; b++) {
      result[rIdx++] = text[i + b];
    }
    result[rIdx] = '\0'; // temporary end of string for measure

    // measure width of string from their start (lineStartIdx) in pixels
    Vector2 size =
        MeasureTextEx(font, &result[lineStartIdx], fontSize, spacing);

    // if leave out of border
    if (size.x > maxWidth) {
      // if in string was space, transfer via him
      if (lastSpaceIdx != -1 && lastSpaceIdx > lineStartIdx) {
        result[lastSpaceIdx] = '\n';
        lineStartIdx = lastSpaceIdx + 1;
        lastSpaceIdx = -1;
      } else {
        // if spaces wasnt (one very long word),
        // forcibly put transfer before current symbol
        rIdx -= bytesProcessed; // roll back record of current symbol
        result[rIdx++] = '\n';  // put transfer

        lineStartIdx = rIdx;
        // record current symbol again, now on new line
        for (int b = 0; b < bytesProcessed; b++) {
          result[rIdx++] = text[i + b];
        }
        result[rIdx] = '\0';
      }
    }

    i += bytesProcessed; // jump to next UTF-8 symbol
  }

  return result;
}

// ---------------------------------------------------------------
// rvsn engine state (scene)
// ---------------------------------------------------------------
static struct {
  Texture2D bg;
  bool has_bg;

  Texture2D text_box;
  bool has_text_box;
} S;

// ---------------------------------------------------------------
// TV overscan (safe area) compensation
// framebuffer is treated as the design space (1280x720);
// ui content is mapped into a smaller centered safe rectangle
// like tiny3d_UserViewport() did in cookie clicker
// (scale 1280/916 and 720/582 -> margins ~47px/~43px on a TV)
// background stays full-bleed
// ---------------------------------------------------------------
#define SAFE_W 0.927f
#define SAFE_H 0.881f

static float safe_ox(void) { return GetScreenWidth() * (1.0f - SAFE_W) * 0.5f; }
static float safe_oy(void) {
  return GetScreenHeight() * (1.0f - SAFE_H) * 0.5f;
}
static float safe_sw(void) { return GetScreenWidth() * SAFE_W; }
static float safe_sh(void) { return GetScreenHeight() * SAFE_H; }
// ---------------------------------------------------------------
// Lua bindings
// ---------------------------------------------------------------

// rvsn.InitWindow(title)
static int l_InitWindow(lua_State *L) {
  const char *title = luaL_checkstring(L, 1);
  InitWindow(0, 0, title);
  return 0;
}

// rvsn.IsGamepadButtonPressed(gamepad,button)
static int l_IsPressedGamepadButton(lua_State *L) {
  int gamepad = luaL_checkinteger(L, 1);
  GamepadButton button = luaL_checkinteger(L, 2);

  bool is_pressed = IsGamepadButtonPressed(gamepad, button);

  lua_pushboolean(L, is_pressed);
  return 1;
}

// rvsn.SetBackground(path)
static int l_SetBackground(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);
  if (S.has_bg)
    UnloadTexture(S.bg);
  S.bg = LoadTexture(path);
  S.has_bg = 1;
  return 0;
}

// rvsn.ClearBackgroundScene() -- hide current background
static int l_ClearScene(lua_State *L) {
  if (S.has_bg) {
    UnloadTexture(S.bg);
    S.bg = (Texture2D){0};
  }
  S.has_bg = 0;
  return 0;
}

// rvsn.DrawText(text, x, y, fontSize, r, g, b, a)
static int l_DrawText(lua_State *L) {
  const char *text = luaL_checkstring(L, 1);
  int x = luaL_checkint(L, 2);
  int y = luaL_checkint(L, 3);
  int fontSize = luaL_checkint(L, 4);
  Color c = {luaL_checkint(L, 5), luaL_checkint(L, 6), luaL_checkint(L, 7),
             luaL_optint(L, 8, 255)};
  DrawText(text, (int)(safe_ox() + x * SAFE_W), (int)(safe_oy() + y * SAFE_H),
           (int)(fontSize * SAFE_H), c);
  return 0;
}

// rvsn.SetTextBox(path_to_file) -- show text box from file
static int l_ShowTextBox(lua_State *L) {
  const char *path_to_file = luaL_checkstring(L, 1);
  if (S.has_text_box)
    UnloadTexture(S.text_box);
  S.text_box = LoadTexture(path_to_file);
  S.has_text_box = true;
  return 0;
}
// rvsn.HideTextBox() --hide text box
static int l_HideTextBox(lua_State *L) {
  if (S.has_text_box) {
    UnloadTexture(S.text_box);
    S.text_box = (Texture2D){0};
  }
  S.has_text_box = 0;
  return 0;
}

// rvsn.SetCharacterSprite
static int l_SetCharacterSprite(lua_State *L) {
  int id = luaL_checkint(L, 1);

  if (id < 0 || id >= MAX_CHARACTERS)
    return 0;
  if (chars[id] == NULL)
    return 0;
  const char *path_to_file = luaL_checkstring(L, 2);
  int scale = luaL_checkint(L, 3);
  const char *position = luaL_checkstring(L, 4);

  if (strcmp(position, "left") == 0) {
    chars[id]->character_position = LEFT;
  } else if (strcmp(position, "center") == 0) {
    chars[id]->character_position = CENTER;
  } else if (strcmp(position, "right") == 0) {
    chars[id]->character_position = RIGHT;
  }

  if (chars[id]->has_sprite)
    UnloadTexture(chars[id]->sprite);
  chars[id]->sprite = LoadTexture(path_to_file);
  SetTextureFilter(chars[id]->sprite, TEXTURE_FILTER_BILINEAR);
  chars[id]->sprite_scale = scale;
  chars[id]->has_sprite = true;

  return 0;
}

// rvsn.HideCharacterSprite
static int l_HideCharacterSprite(lua_State *L) {
  int id = luaL_checkint(L, 1);
  if (chars[id]->has_sprite) {
    UnloadTexture(chars[id]->sprite);
    chars[id]->has_sprite = false;
  }
  return 0;
}

// ---------------------------------------------------------------
// enum GamepadButton as constants rvsn.* for Lua
// ---------------------------------------------------------------
static void lua_register_gamepad_buttons(lua_State *L) {
  // rvsn.GAMEPAD_BUTTON_*
  lua_pushinteger(L, 0);
  lua_setfield(L, -2, "GAMEPAD_BUTTON_UNKNOWN"); // 0  Unknown button

  lua_pushinteger(L, 1);
  lua_setfield(L, -2, "GAMEPAD_BUTTON_LEFT_FACE_UP"); // 1  DPAD up
  lua_pushinteger(L, 2);
  lua_setfield(L, -2, "GAMEPAD_BUTTON_LEFT_FACE_DOWN"); // 2  DPAD down
  lua_pushinteger(L, 3);
  lua_setfield(L, -2, "GAMEPAD_BUTTON_LEFT_FACE_LEFT"); // 3  DPAD left
  lua_pushinteger(L, 4);
  lua_setfield(L, -2, "GAMEPAD_BUTTON_LEFT_FACE_RIGHT"); // 4 DPAD right

  lua_pushinteger(L, 5);
  lua_setfield(L, -2, "GAMEPAD_BUTTON_RIGHT_FACE_DOWN"); // 5 Cross / A
  lua_pushinteger(L, 6);
  lua_setfield(L, -2, "GAMEPAD_BUTTON_RIGHT_FACE_RIGHT"); // 6 Circle / B
  lua_pushinteger(L, 7);
  lua_setfield(L, -2, "GAMEPAD_BUTTON_RIGHT_FACE_LEFT"); // 7 Square / X
  lua_pushinteger(L, 8);
  lua_setfield(L, -2, "GAMEPAD_BUTTON_RIGHT_FACE_UP"); // 8 Triangle / Y

  lua_pushinteger(L, 9);
  lua_setfield(L, -2, "GAMEPAD_BUTTON_LEFT_TRIGGER_1"); // 9  L1 shoulder
  lua_pushinteger(L, 10);
  lua_setfield(L, -2, "GAMEPAD_BUTTON_LEFT_TRIGGER_2"); // 10 L2 trigger
  lua_pushinteger(L, 11);
  lua_setfield(L, -2, "GAMEPAD_BUTTON_RIGHT_TRIGGER_1"); // 11 R1 shoulder
  lua_pushinteger(L, 12);
  lua_setfield(L, -2, "GAMEPAD_BUTTON_RIGHT_TRIGGER_2"); // 12 R2 trigger

  lua_pushinteger(L, 13);
  lua_setfield(L, -2, "GAMEPAD_BUTTON_MIDDLE_LEFT"); // 13 Select
  lua_pushinteger(L, 14);
  lua_setfield(L, -2, "GAMEPAD_BUTTON_MIDDLE"); // 14 PS / home
  lua_pushinteger(L, 15);
  lua_setfield(L, -2, "GAMEPAD_BUTTON_MIDDLE_RIGHT"); // 15 Start

  lua_pushinteger(L, 16);
  lua_setfield(L, -2, "GAMEPAD_BUTTON_LEFT_THUMB"); // 16 L3
  lua_pushinteger(L, 17);
  lua_setfield(L, -2, "GAMEPAD_BUTTON_RIGHT_THUMB"); // 17 R3
}

static const luaL_Reg rvsn_funcs[] = {
    // Basic raylib
    {"InitWindow", l_InitWindow},
    {"DrawText", l_DrawText},

    // Background
    {"SetBackground", l_SetBackground},
    {"ClearBackground", l_ClearScene},

    // TextBox
    {"SetTextBox", l_ShowTextBox},
    {"ClearTextBox", l_HideTextBox},

    // Gamepad
    {"IsGamepadButtonPressed", l_IsPressedGamepadButton},

    // Character
    {"InitCharacter", l_InitCharacter},
    {"SetCharacterSprite", l_SetCharacterSprite},
    {"HideCharacterSprite", l_HideCharacterSprite},
    {"CharacterSay", l_CharacterSay},

    // AUDIO
    {"PlayAudioFile", l_PlayAudioFile},
    {"SetBackgroundMusic", l_SetBackgroundMusic},
    {NULL, NULL}};

int luaopen_rvsn(lua_State *L) {
  luaL_register(L, "rvsn", rvsn_funcs);
  lua_register_gamepad_buttons(L); // constants rvsn.GAMEPAD_BUTTON_*
  return 1;
}

// ---------------------------------------------------------------
// C-side rendering, called from main loop BEFORE Lua's on_draw
// ---------------------------------------------------------------
void rvsn_draw_scene(void) {

  // BACKGROUND DRAWING
  if (!S.has_bg)
    TraceLog(LOG_WARNING, "warning: cant load background");
  Texture2D *bg = &S.bg;
  Rectangle src = {0, 0, (float)bg->width, (float)bg->height};
  Rectangle dst = {0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()};
  DrawTexturePro(*bg, src, dst, (Vector2){0, 0}, 0.0f, WHITE);

  // CHARACTERS DRAWING
  for (int i = 0; i < MAX_CHARACTERS; i++) {
    if (chars[i] != NULL && chars[i]->has_sprite) {
      float scale = (float)chars[i]->sprite_scale / 100.0f;

      float res_scale = scale * (GetScreenHeight() / 720.0f);

      float w = chars[i]->sprite.width * res_scale;
      float h = chars[i]->sprite.height * res_scale;

      float posX = safe_ox() + (safe_sw() - w) / 2.0f;
      switch (chars[i]->character_position) {
      case CENTER:
        posX = safe_ox() + (safe_sw() - w) / 2.0f;
        break;
      case RIGHT:
        posX = safe_ox() + safe_sw() * 0.9f - w;
        break;
      case LEFT:
        posX = safe_ox() + safe_sw() * 0.1f;
        break;
      }

      float posY = GetScreenHeight() - h;

      DrawTextureEx(chars[i]->sprite, (Vector2){posX, posY}, 0.0f, res_scale,
                    WHITE);
    }
  }

  // TEXT BOX DRAWING
  if (!S.has_text_box)
    TraceLog(LOG_WARNING, "warning: cant load textbox");
  Texture2D *text_box = &S.text_box;
  float screenW = safe_sw();
  float screenH = safe_sh();

  // proportions of source texture
  float aspect = (float)text_box->height / (float)text_box->width;

  float dstW = screenW * 0.9f; // 90% screen width
  float dstH = dstW * aspect;  // height by proportions

  // limitation: box != 1/3 screen
  float maxH = screenH / 3.0f;
  if (dstH > maxH) {
    dstH = maxH;
    dstW = dstH / aspect; // if enter to limit — splush width
  }

  float x = safe_ox() + (screenW - dstW) / 2.0f; // center
  float y = safe_oy() + screenH - dstH;          // locked to down

  DrawTexturePro(
      *text_box,
      (Rectangle){0, 0, (float)text_box->width, (float)text_box->height},
      (Rectangle){x, y, dstW, dstH}, (Vector2){0, 0}, 0.0f, WHITE);

  // TEXT
  float margin = dstH * 0.04f;
  float innerX = x + margin;           // left border of text
  float innerW = dstW - margin * 2.0f; // available width of line
  float cursorY = y + margin;          // write from up to down

  float res_scale = safe_sh() / 720.0f;
  int txt_size = (int)(32 * res_scale);
  int name_size = (int)(22 * res_scale);
  int line_h = (int)(txt_size * 1.2f); // line spacing

  if (say_text != NULL && say_name[0] != '\0') {
    // draw name little bit upper or with small indentation gold/white color
    DrawTextEx(GetFontDefault(), say_name, (Vector2){innerX, cursorY},
               name_size, 1.0f, say_name_color);

    // move cursor Y down after name (size of name font + small indentation)
    cursorY += (int)(name_size * 1.4f);
  }

  char *wrappedText =
      TextWordWrap(say_text, innerW, GetFontDefault(), txt_size, 1.0f);

  DrawTextEx(GetFontDefault(), wrappedText, (Vector2){innerX, cursorY},
             txt_size, 1.0f, WHITE);
  free(wrappedText);
}
