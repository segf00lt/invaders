#include "invaders.h"


/* globals */


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
    assert(gp->entities_allocated < MAX_ENTITIES);
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
    ENTITY_FLAG_COLLIDE |
    ENTITY_FLAG_APPLY_FRICTION |
    ENTITY_FLAG_DRAW_SPRITE |
    ENTITY_FLAG_HAS_MISSILE_LAUNCHER |
    //ENTITY_FLAG_DRAW_BOUNDS |
    0;

  ep->update_order = ENTITY_ORDER_FIRST;
  ep->draw_order = ENTITY_ORDER_FIRST;

  ep->pos = PLAYER_INITIAL_OFFSCREEN_POS;

  ep->missile_launcher.cooldown_period = PLAYER_MISSILE_LAUNCHER_COOLDOWN;
  ep->missile_launcher.initial_vel = PLAYER_MISSILE_VELOCITY;
  ep->missile_launcher.missile_size = PLAYER_MISSILE_SIZE;
  ep->missile_launcher.missile_color = PLAYER_MISSILE_COLOR;
  ep->missile_launcher.damage_mask = PLAYER_MISSILE_DAMAGE_MASK;
  ep->missile_launcher.damage_amount = PLAYER_MISSILE_DAMAGE;

  ep->health = PLAYER_HEALTH;

  ep->sprite = gp->player_texture;
  ep->sprite_tint = WHITE;
  ep->sprite_scale = 1.0;

  ep->bounds_color = RED;

  ep->half_size =
    (Vector2){ (float)ep->sprite.width * 0.5, (float)ep->sprite.height * 0.5 };
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
  ep->formation_slot_size =
    (Vector2){ gp->invader_texture.width + ENEMY_FORMATION_SPACING.x, gp->invader_texture.height + ENEMY_FORMATION_SPACING.y };

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
    ENTITY_FLAG_COLLIDE |
    ENTITY_FLAG_DRAW_SPRITE |
    ENTITY_FLAG_HAS_MISSILE_LAUNCHER |
    0;

  ep->control = ENTITY_CONTROL_INVADER_IN_FORMATION;

  ep->formation_id = formation_id;

  ep->update_order = ENTITY_ORDER_LAST;
  ep->draw_order = ENTITY_ORDER_LAST;

  ep->missile_launcher.cooldown_period = INVADER_MISSILE_COOLDOWN;
  ep->missile_launcher.initial_vel = INVADER_MISSILE_VELOCITY;
  ep->missile_launcher.missile_size = INVADER_MISSILE_SIZE;
  ep->missile_launcher.missile_color = INVADER_MISSILE_COLOR;
  ep->missile_launcher.damage_mask = INVADER_MISSILE_DAMAGE_MASK;
  ep->missile_launcher.damage_amount = INVADER_MISSILE_DAMAGE;

  ep->health = INVADER_HEALTH;

  ep->pos = initial_pos;

  ep->sprite = gp->invader_texture;
  ep->sprite_tint = WHITE;
  ep->sprite_scale = 1.0;

  ep->half_size =
    (Vector2){ (float)ep->sprite.width * 0.5, (float)ep->sprite.height * 0.5 };
}

void entity_init_missile_from_launcher(Game *gp, Entity *ep, Missile_launcher launcher) {
  ep->kind = ENTITY_KIND_MISSILE;

  ep->flags =
    ENTITY_FLAG_DYNAMICS |
    ENTITY_FLAG_COLLIDE |
    ENTITY_FLAG_FILL_BOUNDS |
    0;

  ep->control = ENTITY_CONTROL_MISSILE;

  ep->update_order = ENTITY_ORDER_LAST;
  ep->draw_order = ENTITY_ORDER_LAST;

  ep->damage_mask = launcher.damage_mask;
  ep->damage_amount = launcher.damage_amount;

  ep->pos = launcher.initial_pos;

  ep->vel = launcher.initial_vel;

  ep->fill_color = launcher.missile_color;

  ep->half_size =
    Vector2Scale(launcher.missile_size, 0.5);
}

void game_reset(Game *gp) {
  gp->state = GAME_STATE_NONE;
  gp->next_state = GAME_STATE_NONE;

  gp->entity_free_list = 0;
  gp->entities_allocated = 0;
  gp->live_entities = 0;

  memset(gp->entities, 0, sizeof(Entity) * MAX_ENTITIES);

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

    if(IsKeyPressed(KEY_F5) && IsKeyDown(KEY_LEFT_CONTROL)) {
      game_reset(gp);
    }

    if(IsKeyPressed(KEY_F3) && IsKeyDown(KEY_LEFT_CONTROL)) {
      gp->flags ^= GAME_FLAG_HOT_RELOAD;
    }

    if(IsKeyPressed(KEY_F11)) {
      gp->flags  ^= GAME_FLAG_DEBUG_UI;
    }

    if(IsKeyPressed(KEY_F10) && IsKeyDown(KEY_LEFT_CONTROL)) {
      gp->flags  ^= GAME_FLAG_PLAYER_INVINCIBLE;
    }

    int key = GetCharPressed();
    if(key != 0) {
      gp->player_input.pressed_any_key = true;
    }

  } /* input */

  { /* update */

    if(gp->background_y_offset >= WINDOW_HEIGHT) {
      gp->background_y_offset = 0;
    } else {
      gp->background_y_offset -= gp->timestep * gp->background_scroll_speed;
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
            gp->wave_transition_pre_delay_timer += gp->timestep;
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
    }

    gp->live_entities = 0;

    for(Entity_order order = ENTITY_ORDER_FIRST; order < ENTITY_ORDER_MAX; order++) {
      for(s64 i = 0; i < MAX_ENTITIES; i++) {
        Entity *ep = &gp->entities[i];

        if(ep->live == 0 || ep->update_order != order) {
          continue;
        }

        gp->live_entities++;

        { /* entity_update */
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
                  ep->accel.x += -PLAYER_ACCEL;
                } else if(gp->player_input.stop_move_left) {
                  ep->vel.x = 0;
                }

                if(gp->player_input.move_right) {
                  ep->accel.x += PLAYER_ACCEL;
                } else if(gp->player_input.stop_move_right) {
                  ep->vel.x = 0;
                }

                if(ep->received_damage) {
                  ep->received_damage = 0;

                  ep->flags |= ENTITY_FLAG_USE_DAMAGE_BLINK_TINT;
                  ep->damage_blink_high = 1;
                  ep->damage_blink_period = PLAYER_DAMAGE_BLINK_PERIOD;
                  ep->damage_blink_timer = 0;
                  ep->damage_blink_total_time = PLAYER_DAMAGE_BLINK_TOTAL_TIME;
                  ep->damage_blink_sprite_tint = PLAYER_DAMAGE_BLINK_SPRITE_TINT;
                }

                if(gp->flags & GAME_FLAG_PLAYER_INVINCIBLE) {
                  if(ep->health <= PLAYER_HEALTH) {
                    ep->health = PLAYER_HEALTH;
                  }
                }

                if(ep->health <= 0) {
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

                for(int i = 0; i < MAX_ENTITIES; i++) {
                  Entity *colliding_entity = &gp->entities[i];

                  if(colliding_entity->live && ENTITY_KIND_IN_MASK(colliding_entity->kind, ep->damage_mask)) {
                    Rectangle ep_rec =
                    {
                      ep->pos.x - ep->half_size.x,
                      ep->pos.y - ep->half_size.y,
                      2 * ep->half_size.x,
                      2 * ep->half_size.y,
                    };

                    Rectangle colliding_entity_rec =
                    {
                      colliding_entity->pos.x - colliding_entity->half_size.x,
                      colliding_entity->pos.y - colliding_entity->half_size.y,
                      2 * colliding_entity->half_size.x,
                      2 * colliding_entity->half_size.y,
                    };

                    if(CheckCollisionRecs(ep_rec, colliding_entity_rec)) {
                      colliding_entity->health -= ep->damage_amount;
                      colliding_entity->received_damage = 1;
                      entity_die(gp, ep);
                      goto entity_update_end;
                    }

                  }

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
                }

                if(enemies_count > 0) {
                  while(number_of_shooting_invaders < MAX_SHOOTING_INVADERS_PER_FORMATION && number_of_shooting_invaders < enemies_count) {
                    int index = GetRandomValue(0, enemies_count - 1);

                    Entity *check = enemies[index];

                    if(!check->missile_launcher.shooting) {
                      number_of_shooting_invaders++;
                      if(FloatEquals(check->missile_launcher.cooldown_timer, 0.0f) && GetRandomValue(0, 1) != 0) {
                        check->missile_launcher.shooting = 1;

                        if(GetRandomValue(0, 1) == 0) {
                          check->enemy_total_shooting_time = INVADER_TOTAL_SHOOTING_TIME_LONG;
                        } else {
                          check->enemy_total_shooting_time = INVADER_TOTAL_SHOOTING_TIME_SHORT;
                        }
                      }
                    }

                  }
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
                  gp->score += 5;
                  entity_die(gp, ep);
                  goto entity_update_end;
                }

                if(ep->enemy_total_shooting_time <= 0) {
                  ep->missile_launcher.shooting = 0;
                  ep->enemy_total_shooting_time = 0;
                } else {
                  ep->enemy_total_shooting_time -= gp->timestep;
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

          if(ep->flags & ENTITY_FLAG_HAS_MISSILE_LAUNCHER) {

            if(gp->state == GAME_STATE_MAIN_LOOP) {
              if(ep->missile_launcher.cooldown_timer >= ep->missile_launcher.cooldown_period) {
                ep->missile_launcher.cooldown_timer = 0;

                if(FloatEquals(ep->missile_launcher.cooldown_timer, 0.0f)) {
                  if(ep->missile_launcher.shooting) {
                    Entity *missile = entity_spawn(gp);
                    ep->missile_launcher.initial_pos =
                      Vector2Add(ep->pos, (Vector2){ 0, -ep->missile_launcher.missile_size.y });
                    entity_init_missile_from_launcher(gp, missile, ep->missile_launcher);
                  }
                }

              } else {
                ep->missile_launcher.cooldown_timer += gp->timestep;
              }
            }

          }

          if(ep->flags & ENTITY_FLAG_BLINK_TEXT) {
            if(ep->blink_timer >= ep->blink_period) {
              ep->flags ^= ENTITY_FLAG_DRAW_TEXT;
              ep->blink_timer = 0;
            } else {
              ep->blink_timer += gp->timestep;
            }
          }

entity_update_end:
          PASS; // apparently just placing a label somewhere isn't legal
        } /* entity_update */

      } /* update entities */
    }

  } /* update */

  { /* draw */
    BeginDrawing();

    ClearBackground(BLACK);

    DrawTextureRec(
        gp->background_texture,
        (Rectangle){ 0, gp->background_y_offset, WINDOW_WIDTH, WINDOW_HEIGHT },
        (Vector2){0},
        (Color){255, 255, 255, 160});

    for(Entity_order order = ENTITY_ORDER_FIRST; order < ENTITY_ORDER_MAX; order++) {
      for(s64 i = 0; i < MAX_ENTITIES; i++) {
        Entity *ep = &gp->entities[i];

        if(ep->live == 0 || ep->draw_order != order) {
          continue;
        }

        { /* entity_draw */

          if(ep->flags & ENTITY_FLAG_DRAW_SPRITE) {
            Color tint = ep->sprite_tint;

            if(ep->flags & ENTITY_FLAG_USE_DAMAGE_BLINK_TINT) {
              if(ep->damage_blink_high) {
                tint = ep->damage_blink_sprite_tint;
              }
            }

            DrawTextureV(
                ep->sprite,
                Vector2Subtract(ep->pos, ep->half_size),
                tint);
          }

          if(ep->flags & ENTITY_FLAG_DRAW_TEXT) {
            Vector2 text_size = MeasureTextEx(gp->font, ep->text, ep->font_size, ep->font_spacing);
            Vector2 pos = Vector2Subtract(ep->pos, Vector2Scale(text_size, 0.5));
            DrawTextEx(gp->font, ep->text, pos, ep->font_size, ep->font_spacing, ep->text_color);
          }

          if(ep->flags & ENTITY_FLAG_DRAW_BOUNDS) {
            Rectangle rec =
            {
              ep->pos.x - ep->half_size.x,
              ep->pos.y - ep->half_size.y,
              2 * ep->half_size.x,
              2 * ep->half_size.y,
            };

            DrawRectangleLinesEx(rec, 1.0, ep->bounds_color);
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

    { /* HUD and UI */

      /* player health */
      if(gp->player && gp->player->live && gp->player->kind == ENTITY_KIND_PLAYER) {
        const float scale = 0.65;
        const float spacing = 16.0;
        const float width = (float)gp->player_texture.width * scale + spacing;
        Vector2 v =
        {
          (float)WINDOW_WIDTH - width,
          (float)WINDOW_HEIGHT - (float)gp->player_texture.height * scale - spacing,
        };

        for(int i = 0; i < gp->player->health; i++) {
          DrawTextureEx(gp->player_texture, v, 0, scale, WHITE);
          v.x -= width;
        }
      }

      if(gp->state > GAME_STATE_TITLE_SCREEN) {
        char *score_num_str = game_frame_scratch_alloc(gp, 64);
        stbsp_sprintf(score_num_str, "%li", gp->score);
        int w = MeasureText("SCORE", SCORE_LABEL_FONT_SIZE);
        DrawText("SCORE", 10, 10, SCORE_LABEL_FONT_SIZE, RAYWHITE);
        DrawText(score_num_str, 10 + w + 10, 10, SCORE_LABEL_FONT_SIZE, RED);
      }

      if(gp->state == GAME_STATE_GAME_OVER && gp->show_game_over_banner) {
        char *game_over_banner = "GAME OVER";
        int w = MeasureText(game_over_banner, GAME_OVER_BANNER_FONT_SIZE);
        DrawText(game_over_banner, ((float)WINDOW_WIDTH - (float)w) * 0.5, (float)WINDOW_HEIGHT * 0.4, GAME_OVER_BANNER_FONT_SIZE, INVADER_MISSILE_COLOR);
      }

    } /* HUD and UI */

    if(gp->flags & GAME_FLAG_DEBUG_UI) { /* debug overlay */
      char *debug_text = game_frame_scratch_alloc(gp, 256);
      char *debug_text_fmt =
        "auto_hot_reload: %s\n"
        "player_is_invincible: %s\n"
        "frame time: %.3f\n"
        "live entities count: %li\n"
        "most entities allocated: %li\n"
        "game state: %s";
      stbsp_sprintf(debug_text,
          debug_text_fmt,
          (gp->flags & GAME_FLAG_HOT_RELOAD) ? "on" : "off",
          (gp->flags & GAME_FLAG_PLAYER_INVINCIBLE) ? "on" : "off",
          gp->timestep,
          gp->live_entities,
          gp->entities_allocated,
          Game_state_strings[gp->state]);
      Vector2 debug_text_size = MeasureTextEx(gp->font, debug_text, 22, 1.0);
      DrawText(debug_text, 10, WINDOW_HEIGHT - debug_text_size.y - 10, 20, GREEN);
    } /* debug overlay */

    EndDrawing();
  } /* draw */

  gp->state = gp->next_state;

  gp->frame_index++;

  game_reset_frame_scratch(gp);

}

//int main(void) {
//  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Invaders");
//
//  SetTargetFPS(TARGET_FPS);
//
//  Game *gp = MemAlloc(sizeof(Game));
//  memset(gp, 0, sizeof(Game));
//
//  SetTextLineSpacing(25);
//
//  { /* init game */
//    gp->state = GAME_STATE_NONE;
//    gp->background_scroll_speed = BACKGROUND_SCROLL_SPEED;
//    gp->font = GetFontDefault();
//    arena_init(&gp->frame_scratch);
//
//    gp->background_texture = LoadTexture("sprites/nightsky.png");
//    gp->player_texture = LoadTexture("sprites/ship.png");
//    gp->invader_texture = LoadTexture("sprites/enemy.png");
//  } /* init game */
//
//  while(!WindowShouldClose()) {
//    game_update_and_draw(gp);
//  }
//
//  { /* deinit game */
//    UnloadTexture(gp->background_texture);
//    UnloadTexture(gp->player_texture);
//    UnloadTexture(gp->invader_texture);
//
//  } /* deinit game */
//
//  CloseWindow();
//
//  return 0;
//}
