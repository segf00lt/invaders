#include "invaders.h"


/* generated code */

#include "invaders_sprite_data.c"


/* globals */

b8 number_of_enemies_in_formation_less_than_percent;
b8 player_is_taking_damage = false;

/* function bodies */

force_inline void* game_frame_scratch_alloc(Game *gp, size_t bytes) {
  return arena_push(gp->frame_scratch, bytes, 1);
}

force_inline void game_reset_frame_scratch(Game *gp) {
  arena_clear(gp->frame_scratch);
}

force_inline Entity *entity_spawn(Game *gp) {
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

force_inline void entity_die(Game *gp, Entity *ep) {
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
    ENTITY_FLAG_APPLY_COLLISION |
    ENTITY_FLAG_APPLY_COLLISION_DAMAGE |
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
          .vel = Vector2Scale(PLAYER_MISSILE_VELOCITY, 0.1),
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

  ep->collision_mask = PLAYER_COLLISION_MASK;
  ep->damage_amount = 1;

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

  ep->formation_initial_enemy_count = ENEMY_FORMATION_COLS * ENEMY_FORMATION_ROWS;

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
  ep->draw_order = ENTITY_ORDER_FIRST;

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

void entity_init_health_pack(Game *gp, Entity *ep) {
  ep->kind = ENTITY_KIND_HEALTH_PACK;

  ep->flags =
    ENTITY_FLAG_DYNAMICS |
    ENTITY_FLAG_RECEIVE_COLLISION |
    ENTITY_FLAG_RECEIVE_COLLISION_DAMAGE |
    ENTITY_FLAG_HAS_SPRITE |
    //ENTITY_FLAG_FILL_BOUNDS |
    //ENTITY_FLAG_DRAW_BOUNDS |
    0;

  ep->control = ENTITY_CONTROL_HEALTH_PACK;

  ep->update_order = ENTITY_ORDER_LAST;
  ep->draw_order = ENTITY_ORDER_LAST;

  ep->pos =
    (Vector2) {
      .x = (float)GetRandomValue(HEALTH_PACK_SIZE.x, WINDOW_WIDTH - HEALTH_PACK_SIZE.x),
      .y = HEALTH_PACK_INITIAL_Y,
    };

  ep->vel = HEALTH_PACK_VELOCITY;

  ep->bounds_color = RED;

  ep->sprite = SPRITE_HEALTH;
  ep->sprite_tint = WHITE;
  ep->sprite_scale = HEALTH_PACK_SPRITE_SCALE;

  ep->half_size =
    Vector2Scale(HEALTH_PACK_SIZE, 0.5);
}

void entity_init_shields_pickup(Game *gp, Entity *ep) {
  ep->kind = ENTITY_KIND_SHIELDS_PICKUP;

  ep->flags =
    ENTITY_FLAG_DYNAMICS |
    ENTITY_FLAG_RECEIVE_COLLISION |
    ENTITY_FLAG_RECEIVE_COLLISION_DAMAGE |
    ENTITY_FLAG_HAS_SPRITE |
    //ENTITY_FLAG_FILL_BOUNDS |
    //ENTITY_FLAG_DRAW_BOUNDS |
    0;

  ep->control = ENTITY_CONTROL_SHIELDS_PICKUP;

  ep->update_order = ENTITY_ORDER_LAST;
  ep->draw_order = ENTITY_ORDER_LAST;

  ep->pos =
    (Vector2) {
      .x = (float)GetRandomValue(SHIELDS_PICKUP_SIZE.x, WINDOW_WIDTH - SHIELDS_PICKUP_SIZE.x),
      .y = SHIELDS_PICKUP_INITIAL_Y,
    };

  ep->vel = SHIELDS_PICKUP_VELOCITY;

  ep->bounds_color = RED;

  ep->sprite = SPRITE_SHIELDS_PICKUP;
  ep->sprite_tint = WHITE;
  ep->sprite_scale = SHIELDS_PICKUP_SPRITE_SCALE;

  ep->half_size =
    Vector2Scale(SHIELDS_PICKUP_SIZE, 0.5);
}

void entity_init_double_missile_pickup(Game *gp, Entity *ep) {
  ep->kind = ENTITY_KIND_DOUBLE_MISSILE_PICKUP;

  ep->flags =
    ENTITY_FLAG_DYNAMICS |
    ENTITY_FLAG_RECEIVE_COLLISION |
    ENTITY_FLAG_RECEIVE_COLLISION_DAMAGE |
    ENTITY_FLAG_HAS_SPRITE |
    //ENTITY_FLAG_FILL_BOUNDS |
    //ENTITY_FLAG_DRAW_BOUNDS |
    0;

  ep->control = ENTITY_CONTROL_DOUBLE_MISSILE_PICKUP;

  ep->update_order = ENTITY_ORDER_LAST;
  ep->draw_order = ENTITY_ORDER_LAST;

  ep->pos =
    (Vector2) {
      .x = (float)GetRandomValue(DOUBLE_MISSILE_PICKUP_SIZE.x, WINDOW_WIDTH - DOUBLE_MISSILE_PICKUP_SIZE.x),
      .y = DOUBLE_MISSILE_PICKUP_INITIAL_Y,
    };

  ep->vel = DOUBLE_MISSILE_PICKUP_VELOCITY;

  ep->bounds_color = RED;

  ep->sprite = SPRITE_DOUBLE_MISSILE_PICKUP;
  ep->sprite_tint = WHITE;
  ep->sprite_scale = DOUBLE_MISSILE_PICKUP_SPRITE_SCALE;

  ep->half_size =
    Vector2Scale(DOUBLE_MISSILE_PICKUP_SIZE, 0.5);
}

void invader_go_kamikaze(Game *gp, Entity *ep) {
  ep->formation_id = 0;
  ep->control = ENTITY_CONTROL_INVADER_KAMIKAZE;

  ep->flags &=
    ~ENTITY_FLAG_DYNAMICS |
    ~0;

  ep->flags |=
    ENTITY_FLAG_APPLY_COLLISION |
    ENTITY_FLAG_APPLY_COLLISION_DAMAGE |
    ENTITY_FLAG_DIE_ON_APPLY_COLLISION |
    0;

  ep->kamikaze_orbit_angular_vel = 12.0f;
  ep->kamikaze_orbit_radius = 120;
  ep->kamikaze_orbit_center = (Vector2){ ep->pos.x, ep->pos.y + ep->kamikaze_orbit_radius };
  ep->kamikaze_orbit_arm = (Vector2){ 0, -ep->kamikaze_orbit_radius };

  ep->vel = (Vector2){0};
  ep->collision_mask = INVADER_KAMIKAZE_COLLISION_MASK;
  ep->damage_amount = INVADER_KAMIKAZE_DAMAGE;
  ep->half_size = Vector2Scale(INVADER_BOUNDS_SIZE, 0.5f*1.2f);
  ep->sprite = SPRITE_INVADER_IDLE;
}

Vector2 Vector2RotateStepFixedRadius(Vector2 P, Vector2 C, float delta, float r) {
  Vector2 v = Vector2Subtract(P, C);

  Vector2 v_rot =
  {
    v.x + delta * -v.y,
    v.y + delta *  v.x,
  };

  float len = Vector2Length(v_rot);

  if(len > 0.0f) {
    v_rot = Vector2Scale(v_rot, (1 / len) * r);
  }

  return Vector2Add(C, v_rot);
}

Vector2 Vector2RotateStepFixedRadiusVec(Vector2 v, float delta, float r) {
  Vector2 v_rot =
  {
    v.x + delta * -v.y,
    v.y + delta *  v.x,
  };

  float len = Vector2Length(v_rot);

  if(len > 0.0f) {
    v_rot = Vector2Scale(v_rot, (1 / len) * r);
  }

  return v_rot;
}

force_inline void sprite_update(Game *gp, Sprite *sp) {
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

force_inline Sprite_frame sprite_current_frame(Sprite sp) {
  s32 abs_cur_frame = sp.first_frame + sp.cur_frame;

  if(sp.flags & SPRITE_FLAG_REVERSE) {
    abs_cur_frame = sp.last_frame - sp.cur_frame;
  }

  Sprite_frame frame = __sprite_frames[abs_cur_frame];

  return frame;
}

force_inline b32 sprite_at_keyframe(Sprite sp, s32 keyframe) {
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

force_inline void draw_sprite(Game *gp, Sprite sp, Vector2 pos, Color tint) {
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

  arena_clear(gp->frame_scratch);

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

          PauseSound(gp->invader_missile_sound);
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

          ResumeSound(gp->invader_missile_sound);
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

#ifndef OS_WEB
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
#endif

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
          bool advance_to_next_wave = true;
          int live_enemies = 0;

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
              advance_to_next_wave = false;
              live_enemies++;
            }

            if(ep->kind == ENTITY_KIND_HEALTH_PACK) {
              advance_to_next_wave = false;
            }

            if(ep->kind == ENTITY_KIND_SHIELDS_PICKUP) {
              advance_to_next_wave = false;
            }

          }

          if(!player_is_alive) {
            gp->wave_counter = 1;
            gp->next_state = GAME_STATE_GAME_OVER;
          } else if(advance_to_next_wave) {
            gp->wave_counter++;
            gp->next_state = GAME_STATE_WAVE_TRANSITION;
          }

          if(gp->wave_counter >= 1) {
            if(player_is_alive && live_enemies > 1) {

              // TODO how should we control when the player gets some health?
              if(gp->time_since_last_health_spawned >= 10) {
                {
                  Entity *health_pack = entity_spawn(gp);
                  entity_init_health_pack(gp, health_pack);
                }
                gp->time_since_last_health_spawned = 0;
              } else {
                gp->time_since_last_health_spawned += gp->timestep;
              }

              if(gp->time_since_last_double_missile_spawned >= 10) {
                {
                  Entity *double_missile_pickup = entity_spawn(gp);
                  entity_init_double_missile_pickup(gp, double_missile_pickup);
                }
                gp->time_since_last_double_missile_spawned = 0;
              } else {
                gp->time_since_last_double_missile_spawned += gp->timestep;
              }

              if(gp->time_since_last_shields_spawned >= 20) {
                {
                  Entity *shields_pickup = entity_spawn(gp);
                  entity_init_shields_pickup(gp, shields_pickup);
                }
                gp->time_since_last_shields_spawned = 0;
              } else {
                gp->time_since_last_shields_spawned += gp->timestep;
              }

            }
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
    }

    if(gp->background_y_offset >= gp->background_texture.height) {
      gp->background_y_offset -= gp->background_texture.height;
    } else {
      gp->background_y_offset += gp->timestep * gp->background_scroll_speed;
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

                  SetSoundPan(gp->player_damage_sound, Normalize(ep->pos.x, WINDOW_WIDTH, 0));
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
                  Particle_emitter emitter = ep->particle_emitter;
                  emitter.particle.sprite.fps += GetRandomValue(-3, 0);
                  emitter.particle.pos = ep->pos;
                  emitter.particle.vel =
                    Vector2Add(emitter.particle.vel, (Vector2){2*(float)GetRandomValue(-2, 2),(float)GetRandomValue(-10, 0)});

                  particle_emit(gp, emitter);
                }

              } break;
            case ENTITY_CONTROL_HEALTH_PACK:
              {
                if(ep->received_damage > 0) {
                  if(gp->player->health < PLAYER_HEALTH) {
                    gp->player->health++;
                  }
                  // TODO health sound

                  SetSoundPan(gp->invader_die_sound, Normalize(ep->pos.x, WINDOW_WIDTH, 0));
                  PlaySound(gp->invader_die_sound);
                  entity_die(gp, ep);
                  goto entity_update_end;
                }

                if(ep->pos.y > WINDOW_HEIGHT + ep->half_size.y) {
                  entity_die(gp, ep);
                  goto entity_update_end;
                }

              } break;
            case ENTITY_CONTROL_SHIELDS_PICKUP:
              {
                if(ep->received_damage > 0) {
                  if(!(gp->player->flags & ENTITY_FLAG_HAS_SHIELDS)) {
                    gp->player->flags |= ENTITY_FLAG_HAS_SHIELDS;
                    gp->player->shields_time = 0;
                  }

                  SetSoundPan(gp->invader_die_sound, Normalize(ep->pos.x, WINDOW_WIDTH, 0));
                  PlaySound(gp->invader_die_sound);

                  entity_die(gp, ep);
                  goto entity_update_end;
                }

                if(ep->pos.y > WINDOW_HEIGHT + ep->half_size.y) {
                  entity_die(gp, ep);
                  goto entity_update_end;
                }

              } break;
            case ENTITY_CONTROL_DOUBLE_MISSILE_PICKUP:
              {
                if(ep->received_damage > 0) {
                  if(!(gp->player->missile_launcher.flags & MISSILE_LAUNCHER_FLAG_DOUBLE_MISSILES)) {
                    gp->player->missile_launcher.flags |= MISSILE_LAUNCHER_FLAG_DOUBLE_MISSILES;
                    gp->player->missile_launcher.double_missile_time = DOUBLE_MISSILE_PICKUP_DURATION;
                  }

                  SetSoundPan(gp->invader_die_sound, Normalize(ep->pos.x, WINDOW_WIDTH, 0));
                  PlaySound(gp->invader_die_sound);

                  entity_die(gp, ep);
                  goto entity_update_end;
                }

                if(ep->pos.y > WINDOW_HEIGHT + ep->half_size.y) {
                  entity_die(gp, ep);
                  goto entity_update_end;
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

                if(gp->state != GAME_STATE_GAME_OVER) {

                  if(enemies_count <= (float)ep->formation_initial_enemy_count * 0.4) {
                    number_of_enemies_in_formation_less_than_percent = 1;

                    if(ep->formation_trigger_enemy_kamikaze_timer < 0) {
                      ep->formation_trigger_enemy_kamikaze_timer = ENEMY_FORMATION_TRIGGER_KAMIKAZE_TIME;

                      int index = GetRandomValue(0, enemies_count - 1);

                      if(enemies_count > 0) {
                        enemies_count--;

                        Entity *enemy = enemies[index];
                        enemies[index] = enemies[enemies_count];

                        invader_go_kamikaze(gp, enemy);
                      }

                    } else {
                      ep->formation_trigger_enemy_kamikaze_timer -= gp->timestep;
                    }

                  } else {
                    number_of_enemies_in_formation_less_than_percent = 0;
                  }

                  // TODO refactor how enemy formations work
                  if(enemies_count > 0) {

                    //if(ep->formation_trigger_enemy_shoot_delay > 0) {
                    //  ep->formation_trigger_enemy_shoot_delay -= gp->timestep;
                    //} else {
                    //  ep->formation_trigger_enemy_shoot_delay = 0;

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
                    //}

                  } else {
                    entity_die(gp, ep);
                    goto entity_update_end;
                  }
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
                  SetSoundPan(gp->invader_die_sound, Normalize(ep->pos.x, WINDOW_WIDTH, 0));
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
            case ENTITY_CONTROL_INVADER_KAMIKAZE:
              {
                if(ep->health <= 0) {
                  SetSoundPan(gp->invader_die_sound, Normalize(ep->pos.x, WINDOW_WIDTH, 0));
                  PlaySound(gp->invader_die_sound);
                  gp->score += 5;

                  ep->particle_emitter.particle.pos = ep->pos;
                  particle_emit(gp, ep->particle_emitter);

                  entity_die(gp, ep);
                  goto entity_update_end;
                }

                ep->vel =
                  Vector2Scale(Vector2Normalize(Vector2Subtract(gp->player->pos, ep->pos)), INVADER_KAMIKAZE_SPEED);

                Vector2 vel_scale_t = Vector2Scale(ep->vel, gp->timestep);

                ep->kamikaze_orbit_center =
                  Vector2Add(ep->kamikaze_orbit_center, vel_scale_t);

                ep->kamikaze_orbit_arm =
                  Vector2RotateStepFixedRadiusVec(
                      ep->kamikaze_orbit_arm,
                      ep->kamikaze_orbit_angular_vel*gp->timestep,
                      ep->kamikaze_orbit_radius);

                ep->pos =
                  Vector2Add(ep->kamikaze_orbit_center, ep->kamikaze_orbit_arm);

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

          if(ep->flags & ENTITY_FLAG_HAS_SHIELDS) {
            if(ep->shields_time > SHIELDS_TIME) {
              ep->shields_time = 0;
              ep->flags &= ~ENTITY_FLAG_HAS_SHIELDS;
            } else {
              ep->shields_time += gp->timestep;
              ep->received_damage = 0;
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

            if(ep->missile_launcher.flags & MISSILE_LAUNCHER_FLAG_DOUBLE_MISSILES) {
              ep->missile_launcher.double_missile_time -= gp->timestep;
              if(ep->missile_launcher.double_missile_time < 0) {
                ep->missile_launcher.double_missile_time = 0;
                ep->missile_launcher.flags &= ~MISSILE_LAUNCHER_FLAG_DOUBLE_MISSILES;
              }
            }

            if(gp->state == GAME_STATE_MAIN_LOOP) {

              // TODO refactor missile launcher
              if(ep->missile_launcher.cooldown_timer == 0.0f) {
                if(ep->missile_launcher.shooting) {

                  if(ep->missile_launcher.flags & MISSILE_LAUNCHER_FLAG_DOUBLE_MISSILES) {
                    ep->missile_launcher.cooldown_timer = ep->missile_launcher.cooldown_period;

                    // TODO this is a hack

                    Entity *missile1 = entity_spawn(gp);
                    ep->missile_launcher.initial_pos = ep->pos;
                    entity_init_missile_from_launcher(gp, missile1, ep->missile_launcher);

                    missile1->pos.x -= missile1->half_size.x + 10.0f;

                    Entity *missile2 = entity_spawn(gp);
                    ep->missile_launcher.initial_pos = ep->pos;
                    entity_init_missile_from_launcher(gp, missile2, ep->missile_launcher);

                    missile2->pos.x += missile1->half_size.x + 10.0f;

                    SetSoundPan(missile1->missile_sound, Normalize(missile1->pos.x, WINDOW_WIDTH, 0));
                    SetSoundPitch(missile1->missile_sound, Remap(GetRandomValue(0,5), 0, 5, 0.95, 1.0));
                    PlaySound(missile1->missile_sound);

                    SetSoundPan(missile2->missile_sound, Normalize(missile2->pos.x, WINDOW_WIDTH, 0));
                    SetSoundPitch(missile2->missile_sound, Remap(GetRandomValue(0,5), 0, 5, 0.95, 1.0));
                    PlaySound(missile2->missile_sound);

                  } else {
                    ep->missile_launcher.cooldown_timer = ep->missile_launcher.cooldown_period;

                    Entity *missile = entity_spawn(gp);
                    ep->missile_launcher.initial_pos = ep->pos;
                    entity_init_missile_from_launcher(gp, missile, ep->missile_launcher);

                    SetSoundPan(missile->missile_sound, Normalize(ep->pos.x, WINDOW_WIDTH, 0));
                    SetSoundPitch(missile->missile_sound, Remap(GetRandomValue(0,5), 0, 5, 0.95, 1.0));

                    PlaySound(missile->missile_sound);
                  }

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

entity_update_end:;
        } /* entity_update */

      } /* update entities */
    }

    gp->live_particles = 0;

    // TODO interesting particles
    for(int i = 0; i < MAX_PARTICLES; i++) {

      Particle *p = &gp->particles[i];

      if(!p->live) {
        continue;
      }
      gp->live_particles++;

      { /* particle_update */

        {
          p->pos = Vector2Add(p->pos, Vector2Scale(p->vel, gp->timestep));
        }

        {
          Rectangle particle_rec =
          {
            p->pos.x, p->pos.y,
            p->half_size.x * 2, p->half_size.y * 2,
          };

          if(!CheckCollisionRecs(particle_rec, WINDOW_RECT)) {
            p->live = 0;
            continue;
          }
        }

        if(p->flags & PARTICLE_FLAG_HAS_SPRITE) {
          sprite_update(gp, &p->sprite);

          if(p->flags & PARTICLE_FLAG_DIE_WHEN_ANIM_FINISH) {
            if(p->sprite.flags & SPRITE_FLAG_AT_LAST_FRAME) {
              p->live = 0;
            }
          }

        }

      } /* particle_update */

    }

update_end:;
  } /* update */

  { /* draw */
    BeginTextureMode(gp->render_texture);

    ClearBackground(BLACK);

    DrawTextureV(
        gp->background_texture,
        (Vector2){0, gp->background_y_offset + gp->background_texture.height},
        (Color){255, 255, 255, 150});

    DrawTextureV(
        gp->background_texture,
        (Vector2){0, gp->background_y_offset},
        (Color){255, 255, 255, 150});

    DrawTextureV(
        gp->background_texture,
        (Vector2){0, gp->background_y_offset - gp->background_texture.height},
        (Color){255, 255, 255, 150});

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

            if(ep->flags & ENTITY_FLAG_HAS_SHIELDS) {
              Color begin = YELLOW;
              float freq = SHIELDS_COLOR_ANIMATE_FREQ;
              if(ep->shields_time >= SHIELDS_TIME * 0.77) {
                freq *= 2.0;
                begin = WHITE;
              }
              float t = Normalize(sinf(ep->shields_time*freq), -1, 1);
              tint = ColorLerp(begin, MAGENTA, t);
            }

            draw_sprite_ex(gp, ep->sprite, ep->pos, ep->sprite_scale, 0, tint);

          }

          if(ep->flags & ENTITY_FLAG_DRAW_TEXT) {
            Vector2 text_size = MeasureTextEx(gp->font, ep->text, ep->font_size, ep->font_spacing);
            Vector2 pos = Vector2Subtract(ep->pos, Vector2Scale(text_size, 0.5));
            DrawTextEx(gp->font, ep->text, pos, ep->font_size, ep->font_spacing, ep->text_color);
          }

          if(gp->debug_flags & GAME_DEBUG_FLAG_DRAW_ALL_ENTITY_BOUNDS) {
            Rectangle rec =
            {
              ep->pos.x - ep->half_size.x,
              ep->pos.y - ep->half_size.y,
              2 * ep->half_size.x,
              2 * ep->half_size.y,
            };

            DrawRectangleLinesEx(rec, 2.0, ep->bounds_color);
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
        Str8 score_num_str = push_str8f(gp->frame_scratch, "%li", gp->score);
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
    ClearBackground(BLACK);

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
        "invaders will go kamikaze: %s\n"
        "auto_hot_reload: %s\n"
        "player_is_invincible: %s\n"
        "frame time: %.7f\n"
        "live entities count: %li\n"
        "most entities allocated: %li\n"
        "live particles count: %li\n"
        "particle_buf_pos: %i\n"
        "background y offset: %f\n"
        "background texture height: %i\n"
        "screen width: %i\n"
        "screen height: %i\n"
        "game state: %s";
      stbsp_sprintf(debug_text,
          debug_text_fmt,
          number_of_enemies_in_formation_less_than_percent ? "true" : "false",
          (gp->debug_flags & GAME_DEBUG_FLAG_HOT_RELOAD) ? "on" : "off",
          (gp->debug_flags & GAME_DEBUG_FLAG_PLAYER_INVINCIBLE) ? "on" : "off",
          gp->timestep,
          gp->live_entities,
          gp->entities_allocated,
          gp->live_particles,
          gp->particles_buf_pos,
          gp->background_y_offset,
          gp->background_texture.height,
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
