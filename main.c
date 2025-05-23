#include "invaders.h"
#include <dlfcn.h>


#ifdef __MACH__
#define INVADERS_MODULE_PATH "./invaders.dylib"
#else
#define INVADERS_MODULE_PATH "./invaders.so"
#endif


typedef void (*Game_update_and_draw_proc)(Game *);


void load_assets(Game *gp) {
  gp->font = GetFontDefault();

  Image background_image = LoadImage("./sprites/nightsky.png");
  ImageResizeNN(&background_image, background_image.width*BACKGROUND_SCALE, background_image.height*BACKGROUND_SCALE);
  gp->background_texture = LoadTextureFromImage(background_image);

  gp->sprite_atlas = LoadTexture("./aseprite/atlas.png");

  gp->player_missile_sound = LoadSound("./sounds/missile_sound.wav");
  gp->invader_missile_sound = LoadSound("./sounds/invader_missile.wav");
  gp->invader_die_sound = LoadSound("./sounds/invader_damage.wav");
  gp->player_damage_sound = LoadSound("./sounds/player_damage.wav");
  gp->player_die_sound = LoadSound("./sounds/player_die.wav");
  gp->wave_banner_sound = LoadSound("./sounds/wave_banner.wav");
  gp->hyperspace_jump_sound = LoadSound("./sounds/hyperspace_jump.wav");

  gp->render_texture = LoadRenderTexture(WINDOW_WIDTH, WINDOW_HEIGHT);
}

void unload_assets(Game *gp) {
  UnloadRenderTexture(gp->render_texture);

  UnloadTexture(gp->background_texture);
  UnloadTexture(gp->sprite_atlas);

  UnloadSound(gp->player_missile_sound);
  UnloadSound(gp->invader_missile_sound);
  UnloadSound(gp->invader_die_sound);
  UnloadSound(gp->player_damage_sound);
  UnloadSound(gp->player_die_sound);
  UnloadSound(gp->wave_banner_sound);
  UnloadSound(gp->hyperspace_jump_sound);
}

int main(void) {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(1000, 800, "invaders");
  InitAudioDevice();

  SetMasterVolume(GetMasterVolume() * 0.5);

  SetTargetFPS(TARGET_FPS);
  SetTextLineSpacing(10);
  SetTraceLogLevel(LOG_DEBUG);
  SetExitKey(0);

  context_init();

  Game *gp = MemAlloc(sizeof(Game));

  { /* init game */
    memset(gp, 0, sizeof(Game));

    gp->state = GAME_STATE_NONE;
    gp->background_scroll_speed = BACKGROUND_SCROLL_SPEED;
    gp->frame_scratch = arena_alloc();

    load_assets(gp);

    gp->debug_flags |= GAME_DEBUG_FLAG_HOT_RELOAD;
  } /* init game */

  void *game_module = dlopen(INVADERS_MODULE_PATH, RTLD_NOW);
  Game_update_and_draw_proc game_update_and_draw_proc = (Game_update_and_draw_proc)dlsym(game_module, "game_update_and_draw");
  s64 game_module_modtime = GetFileModTime(INVADERS_MODULE_PATH);


  while(!WindowShouldClose()) {
    game_update_and_draw_proc(gp);

    if(gp->debug_flags & GAME_DEBUG_FLAG_HOT_RELOAD) {
      s64 modtime = GetFileModTime(INVADERS_MODULE_PATH);
      if(game_module_modtime != modtime) {
        game_module_modtime = modtime;
        TraceLog(LOG_DEBUG, "reloading game code");

        unload_assets(gp);
        load_assets(gp);

        WaitTime(0.17f);
        ASSERT(!dlclose(game_module));
        game_module = dlopen(INVADERS_MODULE_PATH, RTLD_NOW);
        game_update_and_draw_proc = (Game_update_and_draw_proc)dlsym(game_module, "game_update_and_draw");

      }
    }

  }

  unload_assets(gp);

  CloseWindow();
  CloseAudioDevice();

  return 0;
}
