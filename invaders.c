#include "invaders.h"


/* generated code */

#include "invaders_sprite_data.c"


/* globals */

b8 player_is_taking_damage = false;

/* function bodies */

INLINE void* game_frame_scratch_alloc(Game *gp, size_t bytes) {
  return arena_alloc(&gp->frame_scratch, bytes);
}

INLINE void game_reset_frame_scratch(Game *gp) {
  arena_reset(&gp->frame_scratch);
}

INLINE Entity *entity_spawn(Game *gp) {
  Entity *ep = NULL;

  if(!gp->entity_free_list) {
    ep = &gp->entities[gp->entities_allocated];
    gp->entities_allocated++;
    ASSERT(gp->entities_allocated < MAX_ENTITIES);
  } else {
    ep = gp->entity_free_list;
    gp->entity_free_list = gp->entity_free_list->free_list_next;
  }

  *ep = (Entity){0};
  ep->live = 1;
  ep->genid = gp->frame_index;

  return ep;
}

INLINE void entity_die(Game *gp, Entity *ep) {
  ep->free_list_next = gp->entity_free_list;
  gp->entity_free_list = ep;
  ep->live = 0;
}

void entity_init_title_banner(Game *gp, Entity *ep) {
  ep->kind = ENTITY_KIND_BANNER;

  ep->flags =
    ENTITY_FLAG_DYNAMICS |
    ENTITY_FLAG_DRAW_TEXT |
    0;

  ep->update_order = ENTITY_ORDER_FIRST;
  ep->draw_order = ENTITY_ORDER_LAST;

  ep->pos = TITLE_BANNER_INITIAL_POS;

  ep->text = "INVADERS";
  ep->font_size = TITLE_BANNER_FONT_SIZE;
  ep->font_spacing = TITLE_BANNER_FONT_SPACING;
  ep->text_color = TITLE_BANNER_COLOR;
}

void entity_init_title_screen_hint_text(Game *gp, Entity *ep) {
  ep->kind = ENTITY_KIND_BANNER;

  ep->flags =
    ENTITY_FLAG_DYNAMICS |
    ENTITY_FLAG_DRAW_TEXT |
    ENTITY_FLAG_BLINK_TEXT |
    0;

  ep->update_order = ENTITY_ORDER_FIRST;
  ep->draw_order = ENTITY_ORDER_LAST;

  ep->pos = TITLE_HINT_INITIAL_POS;

  ep->text = "press any key to start";
  ep->font_size = TITLE_HINT_FONT_SIZE;
  ep->font_spacing = TITLE_HINT_FONT_SPACING;
  ep->text_color = TITLE_HINT_COLOR;

  ep->blink_period = TITLE_HINT_BLINK_PERIOD;
}

void entity_init_player(Game *gp, Entity *ep) {
  ep->kind = ENTITY_KIND_PLAYER;

  ep->flags =
    ENTITY_FLAG_DYNAMICS |
    ENTITY_FLAG_RECEIVE_COLLISION |
    ENTITY_FLAG_APPLY_FRICTION |
    ENTITY_FLAG_HAS_SPRITE |
    ENTITY_FLAG_RECEIVE_COLLISION_DAMAGE |
    ENTITY_FLAG_HAS_MISSILE_LAUNCHER |
    //ENTITY_FLAG_DRAW_BOUNDS |
    0;

  ep->update_order = ENTITY_ORDER_LAST;
  ep->draw_order = ENTITY_ORDER_FIRST;

  ep->pos = PLAYER_INITIAL_OFFSCREEN_POS;

  ep->missile_launcher =
  (Missile_launcher){
    .missile_entity_flags =
      ENTITY_FLAG_HAS_SPRITE |
      ENTITY_FLAG_HAS_PARTICLE_EMITTER |
      0,

    .cooldown_period = PLAYER_MISSILE_LAUNCHER_COOLDOWN,
    .spawn_offset = PLAYER_MISSILE_SPAWN_OFFSET,
    .initial_vel = PLAYER_MISSILE_VELOCITY,
    .missile_size = PLAYER_MISSILE_SIZE,
    .missile_color = PLAYER_MISSILE_COLOR,
    .collision_mask = PLAYER_MISSILE_COLLISION_MASK,
    .damage_amount = PLAYER_MISSILE_DAMAGE,
    .missile_sound = &gp->player_missile_sound,

    .particle_emitter = {
      .particle = {
        .flags =
          PARTICLE_FLAG_HAS_SPRITE |
          PARTICLE_FLAG_DIE_WHEN_ANIM_FINISH |
          0,
        .sprite = SPRITE_ORANGE_FIRE_PARTICLE_MAIN,
        .sprite_tint = WHITE,
        .sprite_scale = ORANGE_FIRE_PARTICLE_SPRITE_SCALE,
        .vel = Vector2Scale(PLAYER_MISSILE_VELOCITY, 0.2),
        .half_size = { 8, 8 },
      },
    },

    .sprite = SPRITE_SHIP_MISSILE,
    .sprite_tint = WHITE,
    .sprite_scale = PLAYER_MISSILE_SPRITE_SCALE,
  };

  ep->health = PLAYER_HEALTH;

  ep->sprite = SPRITE_SHIP_IDLE;
  ep->sprite_tint = WHITE;
  ep->sprite_scale = PLAYER_SPRITE_SCALE;

  ep->bounds_color = RED;

  ep->half_size =
    Vector2Scale(PLAYER_BOUNDS_SIZE, 0.5);
}

void entity_init_wave_banner(Game *gp, Entity *ep) {
  ep->kind = ENTITY_KIND_BANNER;

  ep->flags =
    ENTITY_FLAG_DRAW_TEXT |
    0;

  ep->update_order = ENTITY_ORDER_FIRST;
  ep->draw_order = ENTITY_ORDER_LAST;

  ep->pos = WAVE_BANNER_POS;

  ep->text = gp->wave_banner_buf;
  stbsp_sprintf(ep->text, "WAVE %i", gp->wave_counter);
  ep->font_size = WAVE_BANNER_FONT_SIZE;
  ep->font_spacing = WAVE_BANNER_FONT_SPACING;
  ep->text_color = WAVE_BANNER_COLOR;
}

void entity_init_enemy_formation(Game *gp, Entity *ep) {
  gp->current_formation_id++;

  ep->formation_id = gp->current_formation_id;
  // TODO do a better job of spacing enemies in the formation
  ep->formation_slot_size =
    Vector2Add(INVADER_BOUNDS_SIZE, ENEMY_FORMATION_SPACING);

  ep->kind = ENTITY_KIND_FORMATION;

  ep->flags =
    ENTITY_FLAG_DYNAMICS |
    //ENTITY_FLAG_DRAW_BOUNDS |
    0;

  ep->control = ENTITY_CONTROL_ENEMY_FORMATION_ENTER_LEVEL;

  ep->half_size =
    (Vector2){
      ENEMY_FORMATION_COLS * ep->formation_slot_size.x * 0.5,
      ENEMY_FORMATION_ROWS * ep->formation_slot_size.y * 0.5,
    };

  ep->update_order = ENTITY_ORDER_FIRST;
  ep->draw_order = ENTITY_ORDER_FIRST;

  ep->pos = ENEMY_FORMATION_INITIAL_POS;

  ep->bounds_color = GREEN;
}

void entity_init_invader_in_formation(Game *gp, Entity *ep, u64 formation_id, Vector2 initial_pos) {
  ep->kind = ENTITY_KIND_INVADER;

  ep->flags =
    ENTITY_FLAG_DYNAMICS |
    ENTITY_FLAG_RECEIVE_COLLISION |
    ENTITY_FLAG_HAS_SPRITE |
    ENTITY_FLAG_RECEIVE_COLLISION_DAMAGE |
    ENTITY_FLAG_HAS_MISSILE_LAUNCHER |
    ENTITY_FLAG_HAS_PARTICLE_EMITTER |
    //ENTITY_FLAG_DRAW_BOUNDS |
    0;

  ep->control = ENTITY_CONTROL_INVADER_IN_FORMATION;

  ep->formation_id = formation_id;

  ep->update_order = ENTITY_ORDER_LAST;
  ep->draw_order = ENTITY_ORDER_LAST;

  ep->missile_launcher =
    (Missile_launcher){
      .missile_entity_flags =
        ENTITY_FLAG_HAS_SPRITE |
        ENTITY_FLAG_HAS_PARTICLE_EMITTER |
        0,

      .cooldown_period = INVADER_MISSILE_COOLDOWN,
      .spawn_offset = INVADER_MISSILE_SPAWN_OFFSET,
      .initial_vel = INVADER_MISSILE_VELOCITY,
      .missile_size = INVADER_MISSILE_SIZE,
      .missile_color = INVADER_MISSILE_COLOR,
      .collision_mask = INVADER_MISSILE_COLLISION_MASK,
      .damage_amount = INVADER_MISSILE_DAMAGE,
      .missile_sound = &gp->invader_missile_sound,

      .particle_emitter = {
        .particle = {
          .flags =
            PARTICLE_FLAG_HAS_SPRITE |
            PARTICLE_FLAG_DIE_WHEN_ANIM_FINISH |
            0,
          .sprite = SPRITE_PURPLE_FIRE_PARTICLE_MAIN,
          .sprite_tint = WHITE,
          .sprite_scale = PURPLE_FIRE_PARTICLE_SPRITE_SCALE,
          .vel = Vector2Scale(INVADER_MISSILE_VELOCITY, 0.1),
          .half_size = { 8, 8 },
        },
      },

      .sprite = SPRITE_INVADER_PLASMA_MISSILE,
      .sprite_tint = WHITE,
      .sprite_scale = INVADER_MISSILE_SPRITE_SCALE,
    };

  ep->particle_emitter =
    (Particle_emitter){
      .emit_count = 0,
      .particle = {
        .flags =
          PARTICLE_FLAG_HAS_SPRITE |
          PARTICLE_FLAG_DIE_WHEN_ANIM_FINISH |
          0,
        .half_size = { 16, 16 },
        .sprite = SPRITE_ORANGE_EXPLOSION_MAIN,
        .sprite_tint = WHITE,
        .sprite_scale = ORANGE_EXPLOSION_SCALE,
      },
    };

    ep->health = INVADER_HEALTH;

  ep->pos = initial_pos;

  ep->sprite = SPRITE_INVADER_IDLE;
  ep->sprite_tint = WHITE;
  ep->sprite_scale = INVADER_SPRITE_SCALE;

  ep->bounds_color = GREEN;

  // TODO improve how entities are sized
  ep->half_size =
    Vector2Scale(INVADER_BOUNDS_SIZE, 0.5);
}

void entity_init_missile_from_launcher(Game *gp, Entity *ep, Missile_launcher launcher) {
  ep->kind = ENTITY_KIND_MISSILE;

  ep->flags =
    launcher.missile_entity_flags |
    ENTITY_FLAG_DYNAMICS |
    ENTITY_FLAG_APPLY_COLLISION |
    ENTITY_FLAG_APPLY_COLLISION_DAMAGE |
    ENTITY_FLAG_DIE_ON_APPLY_COLLISION |
    //ENTITY_FLAG_FILL_BOUNDS |
    //ENTITY_FLAG_DRAW_BOUNDS |
    0;

  ep->control = ENTITY_CONTROL_MISSILE;

  ep->update_order = ENTITY_ORDER_FIRST;
  ep->draw_order = ENTITY_ORDER_LAST;

  ep->collision_mask = launcher.collision_mask;
  ep->damage_amount = launcher.damage_amount;

  ep->missile_sound = *launcher.missile_sound;

  ep->pos = Vector2Add(launcher.initial_pos, launcher.spawn_offset);

  ep->vel = launcher.initial_vel;

  ep->fill_color = launcher.missile_color;
  ep->bounds_color = launcher.missile_color;

  ep->particle_emitter = launcher.particle_emitter;

  ep->sprite = launcher.sprite;
  ep->sprite_tint = launcher.sprite_tint;
  ep->sprite_scale = launcher.sprite_scale;

  ep->half_size =
    Vector2Scale(launcher.missile_size, 0.5);
}

INLINE void sprite_update(Game *gp, Sprite *sp) {
  if(!(sp->flags & SPRITE_FLAG_STILL)) {

    ASSERT(sp->fps > 0);

    if(sp->cur_frame < sp->total_frames) {
      sp->frame_counter++;

      if(sp->frame_counter >= (TARGET_FPS / sp->fps)) {
        sp->frame_counter = 0;
        sp->cur_frame++;
      }

    }

    if(sp->cur_frame >= sp->total_frames) {
      if(sp->flags & SPRITE_FLAG_INFINITE_REPEAT) {
        if(sp->flags & SPRITE_FLAG_PINGPONG) {
          sp->cur_frame--;
          sp->flags ^= SPRITE_FLAG_REVERSE;
        } else {
          sp->cur_frame = 0;
        }
      } else {
        sp->cur_frame--;
        sp->flags |= SPRITE_FLAG_AT_LAST_FRAME | SPRITE_FLAG_STILL;
        ASSERT(sp->cur_frame >= 0 && sp->cur_frame < sp->total_frames);
      }
    }

  }

}

INLINE Sprite_frame sprite_current_frame(Sprite sp) {
  s32 abs_cur_frame = sp.first_frame + sp.cur_frame;

  if(sp.flags & SPRITE_FLAG_REVERSE) {
    abs_cur_frame = sp.last_frame - sp.cur_frame;
  }

  Sprite_frame frame = __sprite_frames[abs_cur_frame];

  return frame;
}

INLINE b32 sprite_at_keyframe(Sprite sp, s32 keyframe) {
  b32 result = 0;

  s32 abs_cur_frame = sp.first_frame + sp.cur_frame;

  if(sp.flags & SPRITE_FLAG_REVERSE) {
    abs_cur_frame = sp.last_frame - sp.cur_frame;
  }

  if(abs_cur_frame == keyframe) {
    result = 1;
  }

  return result;
}

INLINE void draw_sprite(Game *gp, Sprite sp, Vector2 pos, Color tint) {
  draw_sprite_ex(gp, sp, pos, 1.0f, 0.0f, tint);
}

void draw_sprite_ex(Game *gp, Sprite sp, Vector2 pos, f32 scale, f32 rotation, Color tint) {
  ASSERT(sp.cur_frame >= 0 && sp.cur_frame < sp.total_frames);

  Sprite_frame frame;

  if(sp.flags & SPRITE_FLAG_REVERSE) {
    frame = __sprite_frames[sp.last_frame - sp.cur_frame];
  } else {
    frame = __sprite_frames[sp.first_frame + sp.cur_frame];
  }

  Rectangle source_rec =
  {
    .x = (float)frame.x,
    .y = (float)frame.y,
    .width = (float)frame.w,
    .height = (float)frame.h,
  };

  Rectangle dest_rec =
  {
    .x = pos.x - scale*0.5f*source_rec.width,
    .y = pos.y - scale*0.5f*source_rec.height,
    .width = scale*source_rec.width,
    .height = scale*source_rec.height,
  };

  if(sp.flags & SPRITE_FLAG_DRAW_MIRRORED_X) {
    source_rec.width *= -1;
  }

  if(sp.flags & SPRITE_FLAG_DRAW_MIRRORED_Y) {
    source_rec.height *= -1;
  }

  DrawTexturePro(gp->sprite_atlas, source_rec, dest_rec, (Vector2){0}, rotation, tint);
}

void particle_emit(Game *gp, Particle_emitter emitter) {
  Particle *p = &gp->particles[gp->particles_buf_pos];
  gp->particles_buf_pos++;
  gp->particles_buf_pos %= MAX_PARTICLES;

  *p = emitter.particle;

  p->live = 1;
}

void game_reset(Game *gp) {
  gp->state = GAME_STATE_NONE;
  gp->next_state = GAME_STATE_NONE;

  gp->flags = 0;

  gp->entity_free_list = 0;
  gp->entities_allocated = 0;
  gp->live_entities = 0;

  gp->particles_buf_pos = 0;
  gp->live_particles = 0;

  memory_set(gp->entities, 0, sizeof(Entity) * MAX_ENTITIES);
  memory_set(gp->particles, 0, sizeof(Particle) * MAX_PARTICLES);

  gp->background_scroll_speed = BACKGROUND_SCROLL_SPEED;

  gp->frame_index = 0;
  gp->score = 0;
  gp->player_genid = 0;
  gp->title_screen_banner = 0;
  gp->title_screen_hint = 0;
  gp->wave_banner = 0;
  gp->player = 0;
  gp->player_genid = 0;
  gp->wave_counter = 0;

  gp->current_formation_id = 0;

  arena_reset(&gp->frame_scratch);

  gp->wave_transition_pre_delay_timer = 0;
  gp->wave_transition_post_delay_timer = 0;
  gp->wave_transition_ramp_timer = 0;
  gp->wave_transition_banner_timer = 0;

  gp->game_over_banner_pre_delay_timer = 0;

  gp->title_screen_scroll_title = 0;
  gp->wave_start_ramped_background_scroll_speed = 0;
  gp->spawned_enemies = 0;
  gp->show_game_over_banner = 0;
  gp->spawned_player = 0;
}

void game_update_and_draw(Game *gp) {
  gp->timestep = Clamp(1.0f/10.0f, 1.0f/TARGET_FPS, GetFrameTime());

  gp->next_state = gp->state;

  // TODO separate key map from input
  { /* input */
    gp->player_input = (Player_input){0};

    if(IsKeyDown(KEY_A)) {
      gp->player_input.move_left = true;
    }

    if(IsKeyDown(KEY_D)) {
      gp->player_input.move_right = true;
    }

    if(IsKeyReleased(KEY_A)) {
      gp->player_input.stop_move_left = true;
    }

    if(IsKeyReleased(KEY_D)) {
      gp->player_input.stop_move_right = true;
    }

    if(IsKeyDown(KEY_J)) {
      gp->player_input.shoot = true;
    }

    if(IsKeyPressed(KEY_ESCAPE)) {
      if(gp->state != GAME_STATE_NONE && gp->state != GAME_STATE_TITLE_SCREEN && gp->state != GAME_STATE_GAME_OVER) {

        if(!(gp->flags & GAME_FLAG_PAUSE)) {
          gp->resume_hint_blink_high = 1;
          gp->resume_hint_blink_timer = 0;

          //PauseSound(gp->invader_missile_sound);
          PauseSound(gp->player_missile_sound);
          PauseSound(gp->invader_die_sound);
          PauseSound(gp->player_damage_sound);
          PauseSound(gp->player_die_sound);
          PauseSound(gp->wave_banner_sound);
          PauseSound(gp->hyperspace_jump_sound);

          for(int i = 0; i < MAX_ENTITIES; i++) {
            Entity *ep = &gp->entities[i];
            if(IsSoundValid(ep->missile_sound)) {
              PauseSound(ep->missile_sound);
            }
          }

        } else {

          //ResumeSound(gp->invader_missile_sound);
          ResumeSound(gp->player_missile_sound);
          ResumeSound(gp->invader_die_sound);
          ResumeSound(gp->player_damage_sound);
          ResumeSound(gp->player_die_sound);
          ResumeSound(gp->wave_banner_sound);
          ResumeSound(gp->hyperspace_jump_sound);

          for(int i = 0; i < MAX_ENTITIES; i++) {
            Entity *ep = &gp->entities[i];
            if(IsSoundValid(ep->missile_sound)) {
              ResumeSound(ep->missile_sound);
            }
          }

        }

        gp->flags ^= GAME_FLAG_PAUSE;

      }
    }

    if(IsKeyPressed(KEY_F5)) {
      game_reset(gp);
    }

    if(IsKeyPressed(KEY_F3)) {
      gp->debug_flags ^= GAME_DEBUG_FLAG_HOT_RELOAD;
    }

    if(IsKeyPressed(KEY_F11)) {
      gp->debug_flags  ^= GAME_DEBUG_FLAG_DEBUG_UI;
    }

    if(IsKeyPressed(KEY_F10)) {
      gp->debug_flags ^= GAME_DEBUG_FLAG_DRAW_ALL_ENTITY_BOUNDS;
    }

    if(IsKeyPressed(KEY_F7)) {
      gp->debug_flags  ^= GAME_DEBUG_FLAG_PLAYER_INVINCIBLE;
    }

    // TODO debug sandbox
    //if(IsKeyPressed(KEY_F4)) {
    //  gp->state = GAME_STATE_DEBUG_SANDBOX;
    //  gp->debug_flags &= ~GAME_DEBUG_FLAG_SANDBOX_LOADED;
    //}

    int key = GetCharPressed();
    if(key != 0) {
      gp->player_input.pressed_any_key = true;
    }

  } /* input */

  { /* update */

    if(gp->flags & GAME_FLAG_PAUSE) {
      ASSERT(gp->state != GAME_STATE_NONE && gp->state != GAME_STATE_TITLE_SCREEN && gp->state != GAME_STATE_GAME_OVER);
      if(gp->player_input.pressed_any_key) {
        gp->flags ^= GAME_FLAG_PAUSE;
      } else {
        goto update_end;
      }
    }

    switch(gp->state) {
      default:
        UNREACHABLE;

      case GAME_STATE_NONE:
        {
          gp->title_screen_banner = entity_spawn(gp);
          entity_init_title_banner(gp, gp->title_screen_banner);

          gp->title_screen_hint = entity_spawn(gp);
          entity_init_title_screen_hint_text(gp, gp->title_screen_hint);

          gp->wave_counter = 1;

          gp->next_state = GAME_STATE_TITLE_SCREEN;

        } break;
      case GAME_STATE_TITLE_SCREEN:
        {
          if(gp->title_screen_scroll_title) {

            if(gp->title_screen_banner->pos.y <= TITLE_BANNER_MIN_Y) {
              entity_die(gp, gp->title_screen_banner);
              gp->title_screen_scroll_title = false;

              gp->next_state = GAME_STATE_SPAWN_PLAYER;
            } else {
              gp->title_screen_banner->pos.y -= gp->timestep * TITLE_BANNER_SCROLL_SPEED;
            }

          } else if(gp->player_input.pressed_any_key) {
            gp->title_screen_scroll_title = true;
            entity_die(gp, gp->title_screen_hint);
          }

        } break;
      case GAME_STATE_SPAWN_PLAYER:
        {
          if(!gp->spawned_player) {
            gp->spawned_player = true;
            gp->player = entity_spawn(gp);
            gp->player_genid = gp->player->genid;
            entity_init_player(gp, gp->player);
          } else if(gp->player->pos.y <= PLAYER_MAIN_Y) {
            gp->player->pos.y = PLAYER_MAIN_Y;
            gp->player->control = ENTITY_CONTROL_PLAYER;
            gp->player->flags |= ENTITY_FLAG_CLAMP_POS_TO_SCREEN;
            gp->wave_start_ramped_background_scroll_speed = false;
            gp->spawned_player = false;

            gp->next_state = GAME_STATE_WAVE_TRANSITION;
          } else {
            gp->player->pos.y -= gp->timestep * PLAYER_ENTER_VIEW_SPEED;
          }
        } break;
      case GAME_STATE_WAVE_TRANSITION:
        {
          if(gp->wave_transition_pre_delay_timer >= WAVE_TRANSITION_PRE_DELAY_TIME) {
            if(gp->wave_start_ramped_background_scroll_speed) {

              if(gp->wave_transition_post_delay_timer >= WAVE_TRANSITION_POST_DELAY_TIME) {
                if(gp->wave_transition_banner_timer >= WAVE_TRANSITION_BANNER_TIME) {

                  gp->wave_transition_banner_timer = 0;
                  gp->wave_transition_post_delay_timer = 0;
                  gp->wave_transition_ramp_timer = 0;
                  gp->wave_transition_pre_delay_timer = 0;

                  gp->wave_start_ramped_background_scroll_speed = false;
                  entity_die(gp, gp->wave_banner);

                  gp->next_state = GAME_STATE_SPAWN_ENEMIES;

                } else {
                  gp->wave_transition_banner_timer += gp->timestep;
                }
              } else {
                gp->wave_transition_post_delay_timer += gp->timestep;

                if(gp->wave_transition_post_delay_timer >= WAVE_TRANSITION_POST_DELAY_TIME) {
                  gp->wave_banner = entity_spawn(gp);
                  entity_init_wave_banner(gp, gp->wave_banner);
                  PlaySound(gp->wave_banner_sound);
                }
              }

            } else {
              gp->wave_transition_ramp_timer += gp->timestep;
              float t = gp->wave_transition_ramp_timer;
              float a = WAVE_TRANSITION_RAMP_ACCEL;
              float r = WAVE_TRANSITION_RAMP_TIME;
              float s = BACKGROUND_SCROLL_SPEED;
              gp->background_scroll_speed = -a * SQUARE(t) + 2*a * r * t + s;

              if(gp->background_scroll_speed <= BACKGROUND_SCROLL_SPEED) {
                gp->wave_start_ramped_background_scroll_speed = true;
              }

            }
          } else {
            if(gp->live_missiles == 0) {
              gp->wave_transition_pre_delay_timer += gp->timestep;
              if(gp->wave_transition_pre_delay_timer >= WAVE_TRANSITION_PRE_DELAY_TIME) {
                PlaySound(gp->hyperspace_jump_sound);
              }
            }
          }

        } break;
      case GAME_STATE_SPAWN_ENEMIES:
        {
          if(!gp->spawned_enemies) {
            Entity *enemy_formation = entity_spawn(gp);
            gp->enemy_formation = enemy_formation;
            entity_init_enemy_formation(gp, enemy_formation);

            Vector2 invader_pos = 
              Vector2Subtract(enemy_formation->pos, enemy_formation->half_size);
            float initial_x = invader_pos.x;

            for(int i = 0; i < ENEMY_FORMATION_ROWS; i++) {

              invader_pos.x = initial_x;

              for(int j = 0; j < ENEMY_FORMATION_COLS; j++) {
                Entity *invader = entity_spawn(gp);
                entity_init_invader_in_formation(gp, invader, enemy_formation->formation_id, Vector2Add(invader_pos, Vector2Scale(enemy_formation->formation_slot_size, 0.5)));
                invader_pos.x += enemy_formation->formation_slot_size.x;
              }

              invader_pos.y += enemy_formation->formation_slot_size.y;

            }

            gp->spawned_enemies = true;

          } else {
            if(gp->enemy_formation->pos.y >= ENEMY_FORMATION_MAIN_Y) {
              gp->enemy_formation->control = ENTITY_CONTROL_ENEMY_FORMATION_MAIN;
              gp->enemy_formation->vel = (Vector2){ ENEMY_FORMATION_STRAFE_VEL_X, 0 };
              if(GetRandomValue(0, 1) > 0) {
                gp->enemy_formation->vel.x *= -1;
              }
              gp->spawned_enemies = false;

              gp->next_state = GAME_STATE_MAIN_LOOP;
            }
          }
        } break;
      case GAME_STATE_MAIN_LOOP:
        {
          bool player_is_alive = false;
          bool all_enemies_are_dead = true;

          for(int i = 0; i < MAX_ENTITIES; i++) {
            Entity *ep = &gp->entities[i];

            if(!ep->live) {
              continue;
            }

            if(ep->kind == ENTITY_KIND_PLAYER) {
              if(ep->genid == gp->player_genid) {
                player_is_alive = true;
              }
            }

            if(ep->kind == ENTITY_KIND_INVADER) {
              all_enemies_are_dead = false;
            }

          }

          if(!player_is_alive) {
            gp->wave_counter = 1;
            gp->next_state = GAME_STATE_GAME_OVER;
          }

          if(all_enemies_are_dead) {
            gp->wave_counter++;
            gp->next_state = GAME_STATE_WAVE_TRANSITION;
          }


        } break;
      case GAME_STATE_GAME_OVER:
        {
          if(!gp->show_game_over_banner) {
            if(gp->game_over_banner_pre_delay_timer >= GAME_OVER_BANNER_PRE_DELAY) {
              gp->show_game_over_banner = true;
            } else {
              gp->game_over_banner_pre_delay_timer += gp->timestep;
            }
          }

          if(gp->show_game_over_banner) {
            if(gp->player_input.pressed_any_key) {
              gp->show_game_over_banner = false;
              gp->game_over_banner_pre_delay_timer = 0;
              gp->score = 0;

              for(int i = 0; i < MAX_ENTITIES; i++) {
                Entity *ep = &gp->entities[i];

                if(ep->live && (ep->kind == ENTITY_KIND_INVADER || ep->kind == ENTITY_KIND_FORMATION || ep->kind == ENTITY_KIND_MISSILE)) {
                  entity_die(gp, ep);
                }
              }

              gp->next_state = GAME_STATE_SPAWN_PLAYER;
            }
          }
        } break;
        // TODO debug sandbox
        //case GAME_STATE_DEBUG_SANDBOX:
        //  {
        //    if(!(gp->debug_flags & GAME_DEBUG_FLAG_SANDBOX_LOADED)) {
        //      // clear and respawn entities in the sandbox
        //    } else {
        //    }
        //  } break;
    } /* switch(gp->state) */

    if(gp->background_y_offset >= WINDOW_HEIGHT) {
      gp->background_y_offset = 0;
    } else {
      gp->background_y_offset -= gp->timestep * gp->background_scroll_speed;
    }

    gp->live_entities = 0;
    gp->live_missiles = 0;

    for(Entity_order order = ENTITY_ORDER_FIRST; order < ENTITY_ORDER_MAX; order++) {
      for(s64 i = 0; i < MAX_ENTITIES; i++) {
        Entity *ep = &gp->entities[i];

        if(ep->live == 0 || ep->update_order != order) {
          continue;
        }

        gp->live_entities++;

        if(ep->kind == ENTITY_KIND_MISSILE) {
          gp->live_missiles++;
        }

        { /* entity_update */
          Entity *colliding_entities[MAX_ENTITIES];
          int colliding_entities_count = 0;
          bool applied_collision = false;

          switch(ep->control) {
            default:
              UNREACHABLE;

            case ENTITY_CONTROL_NONE:
              break;
            case ENTITY_CONTROL_PLAYER:
              {
                if(gp->player_input.shoot) {
                  ep->missile_launcher.shooting = 1;
                } else {
                  ep->missile_launcher.shooting = 0;
                }

                ep->accel = (Vector2){0};

                if(gp->player_input.move_left) {
                  ep->accel.x = -PLAYER_ACCEL;

                  if(!(ep->flags & ENTITY_FLAG_MOVING)) {
                    ep->sprite = SPRITE_SHIP_STRAFE;
                    ep->sprite.flags |= SPRITE_FLAG_DRAW_MIRRORED_X;
                    ep->flags |= ENTITY_FLAG_MOVING;
                  }

                } else if(gp->player_input.stop_move_left) {
                  ep->accel = (Vector2){0};
                  ep->vel.x = 0;
                  ep->sprite = SPRITE_SHIP_STRAFE;
                  ep->sprite.flags |= SPRITE_FLAG_REVERSE;
                  ep->sprite.flags |= SPRITE_FLAG_DRAW_MIRRORED_X;
                  ep->flags &= ~ENTITY_FLAG_MOVING;
                }

                if(gp->player_input.move_right) {
                  ep->accel.x += PLAYER_ACCEL;

                  if(!(ep->flags & ENTITY_FLAG_MOVING)) {
                    ep->sprite = SPRITE_SHIP_STRAFE;
                    ep->flags |= ENTITY_FLAG_MOVING;
                  }

                } else if(gp->player_input.stop_move_right) {
                  ep->accel = (Vector2){0};
                  ep->vel.x = 0;
                  ep->sprite = SPRITE_SHIP_STRAFE;
                  ep->sprite.flags |= SPRITE_FLAG_REVERSE;
                  ep->flags &= ~ENTITY_FLAG_MOVING;
                }

#if 0
                if(ep->damage_blink_total_time > 0) {
                  player_is_taking_damage = true;
                } else {
                  player_is_taking_damage = false;
                }
#endif

                if(ep->received_damage > 0) {

                  PlaySound(gp->player_damage_sound);

                  if(ep->damage_blink_total_time > 0) {
                    ep->received_damage = 0;
                  } else {
                    ep->flags |= ENTITY_FLAG_USE_DAMAGE_BLINK_TINT;
                    ep->damage_blink_high = 1;
                    ep->damage_blink_period = PLAYER_DAMAGE_BLINK_PERIOD;
                    ep->damage_blink_timer = 0;
                    ep->damage_blink_total_time = PLAYER_DAMAGE_BLINK_TOTAL_TIME;
                    ep->damage_blink_sprite_tint = PLAYER_DAMAGE_BLINK_SPRITE_TINT;
                  }
                } else {
                }

                if(gp->debug_flags & GAME_DEBUG_FLAG_PLAYER_INVINCIBLE) {
                  if(ep->health <= PLAYER_HEALTH) {
                    ep->health = PLAYER_HEALTH;
                  }
                }

                if(ep->health <= 0) {
                  PlaySound(gp->player_die_sound);
                  entity_die(gp, ep);
                  goto entity_update_end;
                }

              } break;
            case ENTITY_CONTROL_MISSILE:
              {

                Rectangle ep_rec =
                {
                  ep->pos.x - ep->half_size.x,
                  ep->pos.y - ep->half_size.y,
                  2 * ep->half_size.x,
                  2 * ep->half_size.y,
                };

                if(!CheckCollisionRecs(WINDOW_RECT, ep_rec)) {
                  entity_die(gp, ep);
                  goto entity_update_end;
                }

                if(ep->flags & ENTITY_FLAG_HAS_PARTICLE_EMITTER) {
                  ep->particle_emitter.particle.pos = ep->pos;
                  ep->particle_emitter.particle.vel =
                    Vector2Add(ep->particle_emitter.particle.vel, (Vector2){2*(float)GetRandomValue(-10, 10),});
                  particle_emit(gp, ep->particle_emitter);
                }

              } break;
            case ENTITY_CONTROL_ENEMY_FORMATION_MAIN:
              {

                Entity *enemies[MAX_ENTITIES_PER_FORMATION];
                int enemies_count = 0;

                int number_of_shooting_invaders = 0;

                for(int i = 0; i < MAX_ENTITIES; i++) {
                  Entity *check = &gp->entities[i];

                  if(check->live && check->kind == ENTITY_KIND_INVADER && check->formation_id == ep->formation_id) {
                    enemies[enemies_count++] = check;

                    if(check->missile_launcher.shooting) {
                      number_of_shooting_invaders++;
                    }

                    if(check->pos.x <= check->half_size.x + ENEMY_FORMATION_STRAFE_PADDING) {
                      ep->vel.x = ENEMY_FORMATION_STRAFE_VEL_X;
                    } else if(check->pos.x >= WINDOW_WIDTH - (check->half_size.x + ENEMY_FORMATION_STRAFE_PADDING)) {
                      ep->vel.x = -ENEMY_FORMATION_STRAFE_VEL_X;
                    }

                  }
                }

                for(int i = 0; i < enemies_count; i++) {
                  enemies[i]->vel = ep->vel;
                  if(enemies[i]->enemy_do_spit_animation || enemies[i]->missile_launcher.shooting || enemies[i]->enemy_total_shooting_time > 0) {
                    number_of_shooting_invaders++;
                  }
                }

                // TODO refactor how enemy formations work
                if(enemies_count > 0) {

                  //if(ep->formation_trigger_enemy_shoot_delay > 0) {
                  //  ep->formation_trigger_enemy_shoot_delay -= gp->timestep;
                  //} else {
                  //  ep->formation_trigger_enemy_shoot_delay = 0;

                  if(gp->state != GAME_STATE_GAME_OVER) {
                    while(number_of_shooting_invaders < MAX_SHOOTING_INVADERS_PER_FORMATION && number_of_shooting_invaders < enemies_count) {
                      int index = GetRandomValue(0, enemies_count - 1);

                      Entity *check = enemies[index];

                      number_of_shooting_invaders++;
                      if(check->enemy_total_shooting_time == 0.0f) {
                        if(check->enemy_wait_after_shooting_time == 0.0f && GetRandomValue(0, 1) != 0) {
                          //ep->formation_trigger_enemy_shoot_delay = 0.16f * (float)GetRandomValue(1, 3);
                          check->enemy_do_spit_animation = 1;
                        }
                      }

                    }
                  }
                  //}

                } else {
                  entity_die(gp, ep);
                  goto entity_update_end;
                }

              } break;
            case ENTITY_CONTROL_ENEMY_FORMATION_ENTER_LEVEL:
              {
                ep->vel.y = ENEMY_FORMATION_ENTER_LEVEL_SPEED;

                for(int i = 0; i < MAX_ENTITIES; i++) {
                  Entity *check = &gp->entities[i];

                  if(check->live && check->kind == ENTITY_KIND_INVADER && check->formation_id == ep->formation_id) {
                    check->vel = ep->vel;
                  }
                }
              } break;
            case ENTITY_CONTROL_INVADER_IN_FORMATION:
              {
                if(ep->health <= 0) {
                  PlaySound(gp->invader_die_sound);
                  gp->score += 5;

                  ep->particle_emitter.particle.pos = ep->pos;
                  particle_emit(gp, ep->particle_emitter);

                  entity_die(gp, ep);
                  goto entity_update_end;
                }

                if(ep->enemy_do_spit_animation) {
                  ep->enemy_do_spit_animation = 0;
                  ep->sprite = SPRITE_INVADER_SPITTING;

                  if(GetRandomValue(0, 1) == 0) {
                    ep->enemy_total_shooting_time = INVADER_TOTAL_SHOOTING_TIME_LONG;
                  } else {
                    ep->enemy_total_shooting_time = INVADER_TOTAL_SHOOTING_TIME_SHORT;
                  }
                }

                if(sprite_at_keyframe(ep->sprite, SPRITE_KEYFRAME_INVADER_SPIT_FIRED)) {
                  ep->missile_launcher.shooting = 1;
                  ep->sprite.flags |= SPRITE_FLAG_STILL;
                }

                if(ep->missile_launcher.shooting) {
                  if(ep->enemy_total_shooting_time < 0) {
                    ep->missile_launcher.shooting = 0;
                    ep->enemy_total_shooting_time = 0;
                    ep->sprite = SPRITE_INVADER_IDLE;

                    if(GetRandomValue(0, 1) == 0) {
                      ep->enemy_wait_after_shooting_time = INVADER_WAIT_AFTER_SHOOTING_TIME_LONG;
                    } else {
                      ep->enemy_wait_after_shooting_time = INVADER_WAIT_AFTER_SHOOTING_TIME_SHORT;
                    }

                  } else {
                    ep->enemy_total_shooting_time -= gp->timestep;
                  }
                }


                if(ep->enemy_wait_after_shooting_time < 0) {
                  ep->enemy_wait_after_shooting_time = 0;
                } else {
                  ep->enemy_wait_after_shooting_time -= gp->timestep;
                }

              } break;
          }

          if(ep->flags & ENTITY_FLAG_USE_DAMAGE_BLINK_TINT) {
            if(ep->damage_blink_total_time <= 0) {
              ep->flags ^= ENTITY_FLAG_USE_DAMAGE_BLINK_TINT;
              ep->damage_blink_total_time = 0;
              ep->damage_blink_period = 0;
              ep->damage_blink_timer = 0;
              ep->damage_blink_high = 0;
            } else {
              ep->damage_blink_total_time -= gp->timestep;

              if(ep->damage_blink_timer >= ep->damage_blink_period) {
                ep->damage_blink_timer = 0;
                ep->damage_blink_high = !ep->damage_blink_high;
              } else {
                ep->damage_blink_timer += gp->timestep;
              }
            }
          }

          if(ep->flags & ENTITY_FLAG_DYNAMICS) {
            Vector2 a_times_t = Vector2Scale(ep->accel, gp->timestep);
            ep->vel = Vector2Add(ep->vel, a_times_t);
            ep->pos = Vector2Add(ep->pos, Vector2Add(Vector2Scale(ep->vel, gp->timestep), Vector2Scale(a_times_t, 0.5*gp->timestep)));
          }

          if(ep->flags & ENTITY_FLAG_APPLY_FRICTION) {
            ep->vel = Vector2Subtract(ep->vel, Vector2Scale(ep->vel, FRICTION*gp->timestep));
          }

          if(ep->flags & ENTITY_FLAG_CLAMP_POS_TO_SCREEN) {
            Vector2 pos_min = ep->half_size;
            Vector2 pos_max =
              Vector2Subtract((Vector2){ WINDOW_WIDTH, WINDOW_HEIGHT }, ep->half_size);
            ep->pos = Vector2Clamp(ep->pos, pos_min, pos_max);
          }

          if(ep->flags & ENTITY_FLAG_APPLY_COLLISION) {
            Rectangle ep_rec =
            {
              ep->pos.x - ep->half_size.x,
              ep->pos.y - ep->half_size.y,
              2 * ep->half_size.x,
              2 * ep->half_size.y,
            };

            for(int i = 0; i < MAX_ENTITIES; i++) {
              Entity *colliding_entity = &gp->entities[i];

              if(colliding_entity->live && colliding_entity->flags & ENTITY_FLAG_RECEIVE_COLLISION) {
                Rectangle colliding_entity_rec =
                {
                  colliding_entity->pos.x - colliding_entity->half_size.x,
                  colliding_entity->pos.y - colliding_entity->half_size.y,
                  2 * colliding_entity->half_size.x,
                  2 * colliding_entity->half_size.y,
                };

                if(CheckCollisionRecs(ep_rec, colliding_entity_rec)) {
                  if(entity_kind_in_mask(colliding_entity->kind, ep->collision_mask)) {
                    applied_collision = true;
                    colliding_entities[colliding_entities_count++] = colliding_entity;
                  }
                }

              }

            }

          }

          if(ep->flags & ENTITY_FLAG_APPLY_COLLISION_DAMAGE) {
            for(int i = 0; i < colliding_entities_count; i++) {
              colliding_entities[i]->received_damage = ep->damage_amount;
            }
          }

          if(ep->flags & ENTITY_FLAG_RECEIVE_COLLISION_DAMAGE) {
            if(ep->received_damage > 0) {
              ep->health -= ep->received_damage;
              ep->received_damage = 0;
            }
          }

          if(ep->flags & ENTITY_FLAG_DIE_ON_APPLY_COLLISION) {
            if(applied_collision) {
              entity_die(gp, ep);
              goto entity_update_end;
            }
          }

          if(ep->flags & ENTITY_FLAG_HAS_MISSILE_LAUNCHER) {

            if(gp->state == GAME_STATE_MAIN_LOOP) {

              if(ep->missile_launcher.cooldown_timer == 0.0f) {
                if(ep->missile_launcher.shooting) {
                  ep->missile_launcher.cooldown_timer = ep->missile_launcher.cooldown_period;

                  Entity *missile = entity_spawn(gp);
                  ep->missile_launcher.initial_pos = ep->pos;
                  entity_init_missile_from_launcher(gp, missile, ep->missile_launcher);
                  SetSoundPan(missile->missile_sound, Normalize(ep->pos.x, WINDOW_WIDTH, 0));
                  PlaySound(missile->missile_sound);
                }
              }

              if(ep->missile_launcher.cooldown_timer < 0.0f) {
                ep->missile_launcher.cooldown_timer = 0.0f;
              } else {
                ep->missile_launcher.cooldown_timer -= gp->timestep;
              }

            }

          }

          if(ep->flags & ENTITY_FLAG_HAS_SPRITE) {
            sprite_update(gp, &ep->sprite);
          }

          if(ep->flags & ENTITY_FLAG_BLINK_TEXT) {
            if(ep->blink_timer >= ep->blink_period) {
              ep->flags ^= ENTITY_FLAG_DRAW_TEXT;
              ep->blink_timer = 0;
            } else {
              ep->blink_timer += gp->timestep;
            }
          }

entity_update_end:; // apparently just placing a label somewhere isn't legal
        } /* entity_update */

      } /* update entities */
    }

    gp->live_particles = 0;

    // TODO interesting particles
    for(int i = 0; i < MAX_PARTICLES; i++) { /* update particles */

      Particle *p = &gp->particles[i];

      if(!p->live) {
        continue;
      }
      gp->live_particles++;

      { /* particle_update */
        if(p->flags & PARTICLE_FLAG_HAS_SPRITE) {
          sprite_update(gp, &p->sprite);

          if(p->flags & PARTICLE_FLAG_DIE_WHEN_ANIM_FINISH) {
            if(p->sprite.flags & SPRITE_FLAG_AT_LAST_FRAME) {
              p->live = 0;
            }
          }

        }

      } /* particle_update */

    } /* update particles */

update_end:;
  } /* update */

  { /* draw */
    BeginTextureMode(gp->render_texture);

    ClearBackground(BLACK);

    DrawTextureRec(
        gp->background_texture,
        (Rectangle){ 0, gp->background_y_offset, WINDOW_WIDTH, WINDOW_HEIGHT },
        (Vector2){0},
        (Color){255, 255, 255, 190});

    for(Entity_order order = ENTITY_ORDER_FIRST; order < ENTITY_ORDER_MAX; order++) {
      for(s64 i = 0; i < MAX_ENTITIES; i++) {
        Entity *ep = &gp->entities[i];

        if(ep->live == 0 || ep->draw_order != order) {
          continue;
        }

        { /* entity_draw */

          if(ep->flags & ENTITY_FLAG_HAS_SPRITE) {
            Color tint = ep->sprite_tint;

            if(ep->flags & ENTITY_FLAG_USE_DAMAGE_BLINK_TINT) {
              if(ep->damage_blink_high) {
                tint = ep->damage_blink_sprite_tint;
              }
            }

            draw_sprite_ex(gp, ep->sprite, ep->pos, ep->sprite_scale, 0, tint);

          }

          if(ep->flags & ENTITY_FLAG_DRAW_TEXT) {
            Vector2 text_size = MeasureTextEx(gp->font, ep->text, ep->font_size, ep->font_spacing);
            Vector2 pos = Vector2Subtract(ep->pos, Vector2Scale(text_size, 0.5));
            DrawTextEx(gp->font, ep->text, pos, ep->font_size, ep->font_spacing, ep->text_color);
          }

          if(ep->flags & ENTITY_FLAG_DRAW_BOUNDS || gp->debug_flags & GAME_DEBUG_FLAG_DRAW_ALL_ENTITY_BOUNDS) {
            Rectangle rec =
            {
              ep->pos.x - ep->half_size.x,
              ep->pos.y - ep->half_size.y,
              2 * ep->half_size.x,
              2 * ep->half_size.y,
            };

            DrawRectangleLinesEx(rec, 2.0, ep->bounds_color);
          }

          if(ep->flags & ENTITY_FLAG_FILL_BOUNDS) {
            Rectangle rec =
            {
              ep->pos.x - ep->half_size.x,
              ep->pos.y - ep->half_size.y,
              2 * ep->half_size.x,
              2 * ep->half_size.y,
            };

            DrawRectangleRec(rec, ep->fill_color);
          }

        } /* entity_draw */

      }
    } /* draw entities */

    { /* draw particles */

      for(int i = 0; i < MAX_PARTICLES; i++) {
        Particle *p = &gp->particles[i];

        if(!p->live) {
          continue;
        }

        if(p->flags & PARTICLE_FLAG_HAS_SPRITE) {
          draw_sprite_ex(gp, p->sprite, p->pos, p->sprite_scale, 0, WHITE);
        }

      }

    } /* draw particles */

    { /* HUD and UI */

      /* player health */
      if(gp->player && gp->player->live && gp->player->kind == ENTITY_KIND_PLAYER) {
        Sprite_frame frame = sprite_current_frame(SPRITE_SHIP_HEALTH_ICON);

        float scale = PLAYER_HEALTH_ICON_SPRITE_SCALE;
        float spacing = 3.2;
        float width = (float)frame.w * scale + spacing;
        float half_width = 0.5*(float)frame.w * scale;

        Vector2 pos =
        {
          .x = (float)WINDOW_WIDTH - width + half_width,
          .y = half_width + spacing,
        };

        for(int i = 0; i < gp->player->health; i++) {
          draw_sprite_ex(gp, SPRITE_SHIP_HEALTH_ICON, pos, scale, 0.0f, WHITE);
          pos.x -= width;
        }

      }

      if(gp->state > GAME_STATE_TITLE_SCREEN) {
        Str8 score_num_str = push_str8f(&gp->frame_scratch, "%li", gp->score);
        int w = MeasureText("SCORE", SCORE_LABEL_FONT_SIZE);
        Vector2 pos = { 10, 10 };
        DrawTextEx(gp->font, "SCORE", pos, SCORE_LABEL_FONT_SIZE, SCORE_LABEL_FONT_SIZE/10.0f, RAYWHITE);
        pos.x += w + 10;
        DrawTextEx(gp->font, (char*)score_num_str.s, pos, SCORE_LABEL_FONT_SIZE, SCORE_LABEL_FONT_SIZE/10.0f, RED);
      }

      if(gp->state == GAME_STATE_GAME_OVER && gp->show_game_over_banner) {
        char *game_over_banner = "GAME OVER";
        int w = MeasureText(game_over_banner, GAME_OVER_BANNER_FONT_SIZE);
        DrawText(game_over_banner, ((float)WINDOW_WIDTH - (float)w) * 0.5, (float)WINDOW_HEIGHT * 0.4, GAME_OVER_BANNER_FONT_SIZE, INVADER_MISSILE_COLOR);
      }

      if(gp->flags & GAME_FLAG_PAUSE) {
        DrawRectangleRec(WINDOW_RECT, (Color){ 0, 0, 0, 150 });

        char *pause_banner = "PAUSED";
        int pause_banner_w = MeasureText(pause_banner, PAUSE_BANNER_FONT_SIZE);

        DrawText(pause_banner, ((float)WINDOW_WIDTH - (float)pause_banner_w) * 0.5, (float)WINDOW_HEIGHT * 0.4, PAUSE_BANNER_FONT_SIZE, PAUSE_BANNER_COLOR);

        if(gp->resume_hint_blink_timer >= RESUME_HINT_BLINK_PERIOD) {
          gp->resume_hint_blink_timer = 0;
          gp->resume_hint_blink_high = !gp->resume_hint_blink_high;
        } else {
          gp->resume_hint_blink_timer += gp->timestep;
        }

        if(gp->resume_hint_blink_high) {
          char *resume_hint_banner = "press any key to resume";
          int resume_hint_banner_w = MeasureText(resume_hint_banner, RESUME_HINT_FONT_SIZE);

          DrawText(resume_hint_banner, ((float)WINDOW_WIDTH - (float)resume_hint_banner_w) * 0.5, (float)WINDOW_HEIGHT * 0.50, RESUME_HINT_FONT_SIZE, RESUME_HINT_COLOR);
        }

      }

    } /* HUD and UI */

    EndTextureMode();

    float scale = fminf((float)GetScreenWidth() / WINDOW_WIDTH,
        (float)GetScreenHeight() / WINDOW_HEIGHT);
    int offset_x = (GetScreenWidth() - (int)(WINDOW_WIDTH * scale)) >> 1;
    int offset_y = (GetScreenHeight() - (int)(WINDOW_HEIGHT * scale)) >> 1;

    BeginDrawing();

#if 0
    if(player_is_taking_damage) {
      ClearBackground(RED);
    } else {
      ClearBackground(DARKBLUE);
    }
#endif
    ClearBackground(DARKBLUE);

    {
      Rectangle dest = { (float)offset_x, (float)offset_y, WINDOW_WIDTH * scale, WINDOW_HEIGHT * scale };
      DrawRectangleRec(dest, BLACK);
      DrawTexturePro(gp->render_texture.texture,
          (Rectangle){ 0, 0, (float)gp->render_texture.texture.width, -(float)gp->render_texture.texture.height },
          dest,
          (Vector2){ 0, 0 }, 0.0f, WHITE);
    }

    if(gp->debug_flags & GAME_DEBUG_FLAG_DEBUG_UI) { /* debug overlay */
      char *debug_text = game_frame_scratch_alloc(gp, 256);
      char *debug_text_fmt =
        "auto_hot_reload: %s\n"
        "player_is_invincible: %s\n"
        "frame time: %.7f\n"
        "live entities count: %li\n"
        "most entities allocated: %li\n"
        "live particles count: %li\n"
        "particle_buf_pos: %i\n"
        "screen width: %i\n"
        "screen height: %i\n"
        "game state: %s";
      stbsp_sprintf(debug_text,
          debug_text_fmt,
          (gp->debug_flags & GAME_DEBUG_FLAG_HOT_RELOAD) ? "on" : "off",
          (gp->debug_flags & GAME_DEBUG_FLAG_PLAYER_INVINCIBLE) ? "on" : "off",
          gp->timestep,
          gp->live_entities,
          gp->entities_allocated,
          gp->live_particles,
          gp->particles_buf_pos,
          GetScreenWidth(),
          GetScreenHeight(),
          Game_state_strings[gp->state]);
      Vector2 debug_text_size = MeasureTextEx(gp->font, debug_text, 18, 1.0);
      DrawText(debug_text, 10, GetScreenHeight() - debug_text_size.y - 10, 18, GREEN);
    } /* debug overlay */

    EndDrawing();

  } /* draw */

  gp->state = gp->next_state;

  gp->frame_index++;

  game_reset_frame_scratch(gp);

}
