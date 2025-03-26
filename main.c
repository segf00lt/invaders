#include "invaders.h"
#include <dlfcn.h>


#define INVADERS_MODULE_PATH "./invaders.so"


typedef void (*Game_update_and_draw_proc)(Game *);


int main(void) {
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "invaders 3");

  SetTargetFPS(TARGET_FPS);

  Game *gp = MemAlloc(sizeof(Game));

  { /* init game */
    memset(gp, 0, sizeof(Game));

    gp->state = GAME_STATE_NONE;
    gp->background_scroll_speed = BACKGROUND_SCROLL_SPEED;
    gp->font = GetFontDefault();
    arena_init(&gp->frame_scratch);

    gp->background_texture = LoadTexture("sprites/nightsky.png");
    gp->player_texture = LoadTexture("sprites/ship.png");
    gp->invader_texture = LoadTexture("sprites/enemy.png");

    gp->flags |= GAME_FLAG_HOT_RELOAD;
  } /* init game */

  SetTextLineSpacing(10);
  SetTraceLogLevel(LOG_DEBUG);

  void *game_module = dlopen(INVADERS_MODULE_PATH, RTLD_NOW);
  Game_update_and_draw_proc game_update_and_draw_proc = (Game_update_and_draw_proc)dlsym(game_module, "game_update_and_draw");
  s64 game_module_modtime = GetFileModTime(INVADERS_MODULE_PATH);


  while(!WindowShouldClose()) {
    game_update_and_draw_proc(gp);

    if(gp->flags & GAME_FLAG_HOT_RELOAD) {
      s64 modtime = GetFileModTime(INVADERS_MODULE_PATH);
      if(game_module_modtime != modtime) {
        game_module_modtime = modtime;
        TraceLog(LOG_DEBUG, "reloading game code");
        WaitTime(0.1f);
        assert(!dlclose(game_module));
        game_module = dlopen(INVADERS_MODULE_PATH, RTLD_NOW);
        game_update_and_draw_proc = (Game_update_and_draw_proc)dlsym(game_module, "game_update_and_draw");
      }
    }

  }

  { /* deinit game */
    UnloadTexture(gp->background_texture);
    UnloadTexture(gp->player_texture);
    UnloadTexture(gp->invader_texture);

  } /* deinit game */

  CloseWindow();

  return 0;
}
