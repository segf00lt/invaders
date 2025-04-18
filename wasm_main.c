#include <emscripten/emscripten.h>

#include "invaders.h"


#include "invaders.c"



void load_assets(Game *gp) {
  gp->font = GetFontDefault();
  gp->background_texture = LoadTexture("./sprites/nightsky.png");

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

void wasm_main_loop(void *gp) {
  game_update_and_draw(gp);
}

int main(void) {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(1000, 800, "invaders");
  InitAudioDevice();

  SetTargetFPS(TARGET_FPS);
  SetTextLineSpacing(10);
  SetExitKey(0);

  Game *gp = MemAlloc(sizeof(Game));

  { /* init game */
    memset(gp, 0, sizeof(Game));

    gp->state = GAME_STATE_NONE;
    gp->background_scroll_speed = BACKGROUND_SCROLL_SPEED;
    arena_init(&gp->frame_scratch);

    load_assets(gp);
  } /* init game */

  SetMasterVolume(GetMasterVolume() * 0.5);

  ASSERT(IsAudioDeviceReady());

  emscripten_set_main_loop_arg(wasm_main_loop, (void*)gp, TARGET_FPS, 1);

  unload_assets(gp);

  CloseWindow();
  CloseAudioDevice();

  return 0;
}
