#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <raylib.h>
#include <string.h>
#include <sysmodule/sysmodule.h>

#include "audio_system.h"
#include "character.h"

extern int luaopen_rvsn(lua_State *L);
extern void rvsn_draw_scene(void);

static lua_State *L;
static const char *script_err = NULL;

static void call_hook(const char *name) {
  lua_getglobal(L, name);
  if (lua_isfunction(L, -1)) {
    if (lua_pcall(L, 0, 0, 0) != 0) {
      script_err = lua_tostring(L, -1);
      lua_pop(L, 1);
    }
  } else {
    lua_pop(L, 1);
  }
}

static void lua_add_path(lua_State *L, const char *path) {
  lua_getglobal(L, "package"); // [pkg]
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return;
  }

  lua_getfield(L, -1, "path"); // [pkg, old]
  const char *old = lua_tostring(L, -1);
  char buf[512];
  snprintf(buf, sizeof(buf), "%s;%s", path, old ? old : "");
  lua_pop(L, 1); // [pkg]

  lua_pushstring(L, buf);      // [pkg, new]
  lua_setfield(L, -2, "path"); // pkg.path = new ✓  (таблица ровно в -2)
  lua_pop(L, 1);               // []
}

int main(void) {
  sysModuleLoad(SYSMODULE_FS);
  InitAudioSystem();
  memset(chars, 0, sizeof(chars));

  L = luaL_newstate();
  luaL_openlibs(L);
  lua_add_path(L,
               "/dev_hdd0/simple_test/?.lua;/dev_hdd0/simple_test/?/init.lua;"
               "/dev_hdd0/simple_test/scripts/?.lua");
  luaopen_rvsn(L);

  InitWindow(0, 0, "rvsn");
  SetTargetFPS(60);

  if (luaL_loadfile(L, "/dev_hdd0/simple_test/main.lua") != 0) {
    script_err = lua_tostring(L, -1);
    lua_pop(L, 1);
  } else if (lua_pcall(L, 0, 0, 0) != 0) {
    script_err = lua_tostring(L, -1);
    lua_pop(L, 1);
  }

  while (!WindowShouldClose()) {
    BeginDrawing();
    rvsn_draw_scene();
    call_hook("on_draw");
    if (script_err) {
      ClearBackground(BLACK);
      DrawText("rvsn script error:", 20, 20, 20, RED);
      DrawText(script_err, 20, 60, 16, WHITE);
    }
    EndDrawing();
    call_hook("on_update");
  }

  CloseWindow();
  lua_close(L);
  return 0;
}
