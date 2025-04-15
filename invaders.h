#ifndef INVADERS_H
#define INVADERS_H

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include "basic.h"
#include "arena.h"
#include "str.h"
#include "array.h"
#include "sprite.h"
#include "stb_sprintf.h"


/* constants */
// TODO make the simulated velocities and sizes resolution independant

#define TARGET_FPS 60

#define WINDOW_SCALE 400
#define WINDOW_WIDTH  (4*WINDOW_SCALE)
#define WINDOW_HEIGHT (3*WINDOW_SCALE)

#define WINDOW_RECT ((Rectangle){0, 0, WINDOW_WIDTH, WINDOW_HEIGHT})

#define MAX_ENTITIES 2048
#define MAX_PARTICLES 128

#define MAX_SHOOTING_INVADERS_PER_FORMATION 3

#define MAX_ENTITIES_PER_FORMATION 36

#define BACKGROUND_SCROLL_SPEED ((float)100.0)

#define FRICTION ((float)40.0)

#define TITLE_BANNER_INITIAL_POS ((Vector2){ (float)WINDOW_WIDTH * 0.5, (float)WINDOW_HEIGHT * 0.22 })
#define TITLE_BANNER_FONT_SIZE ((float)120.0f)
#define TITLE_BANNER_FONT_SPACING ((float)12.0f)
#define TITLE_BANNER_COLOR ((Color){ 245, 245, 245, 255 })
#define TITLE_BANNER_SCROLL_SPEED ((float)320)
#define TITLE_BANNER_MIN_Y ((float)-120)

#define TITLE_HINT_INITIAL_POS ((Vector2){ (float)WINDOW_WIDTH * 0.5, (float)WINDOW_HEIGHT * 0.32 })
#define TITLE_HINT_FONT_SIZE ((float)30.0f)
#define TITLE_HINT_FONT_SPACING ((float)3.00f)
#define TITLE_HINT_COLOR ((Color){ 245, 245, 245, 205 })
#define TITLE_HINT_BLINK_PERIOD ((float)0.66)

#define PLAYER_INITIAL_OFFSCREEN_POS ((Vector2){ (float)WINDOW_WIDTH * 0.5, (float)WINDOW_HEIGHT * 1.3f })
#define PLAYER_MAIN_Y ((float)WINDOW_HEIGHT * 0.87f)
#define PLAYER_ENTER_VIEW_SPEED ((float)400)
#define PLAYER_ACCEL ((float)1.6e4)

#define PLAYER_BOUNDS_SIZE ((Vector2){ 60, 70 })
#define PLAYER_SPRITE_SCALE ((float)3.0f)
#define PLAYER_HEALTH_ICON_SPRITE_SCALE ((float)2.0f)

#define PLAYER_COLLISION_MASK \
  ((Entity_kind_mask)(ENTITY_KIND_MASK_HEALTH_PACK | ENTITY_KIND_MASK_SHIELDS_PICKUP | 0))

#define PLAYER_DAMAGE_BLINK_PERIOD ((float)0.03f)
#define PLAYER_DAMAGE_BLINK_TOTAL_TIME ((float)1.2f)
#define PLAYER_DAMAGE_BLINK_SPRITE_TINT (Color){ 255, 0, 0, 255 }

#define PLAYER_MISSILE_LAUNCHER_COOLDOWN ((float)0.32)
#define PLAYER_HEALTH 4

#define PLAYER_MISSILE_COLOR RED
#define PLAYER_MISSILE_SIZE ((Vector2){ 14, 32 })
#define PLAYER_MISSILE_SPAWN_OFFSET ((Vector2){ 0, -0.5f*(PLAYER_MISSILE_SIZE.y + PLAYER_BOUNDS_SIZE.y) })
#define PLAYER_MISSILE_VELOCITY ((Vector2){ 0, -1200 })
#define PLAYER_MISSILE_COLLISION_MASK \
  ((Entity_kind_mask)(ENTITY_KIND_MASK_INVADER |  0))
#define PLAYER_MISSILE_DAMAGE 1
#define PLAYER_MISSILE_SPRITE_SCALE ((float)4.0f)

#define WAVE_TRANSITION_PRE_DELAY_TIME ((float)0.6f)
#define WAVE_TRANSITION_POST_DELAY_TIME ((float)0.3f)
#define WAVE_TRANSITION_RAMP_TIME ((float)2.2f)
#define WAVE_TRANSITION_RAMP_ACCEL ((float)520.0f)
#define WAVE_TRANSITION_BANNER_TIME ((float)2.0f)

#define WAVE_BANNER_POS ((Vector2){ (float)WINDOW_WIDTH * 0.5, (float)WINDOW_HEIGHT * 0.4 })
#define WAVE_BANNER_FONT_SIZE ((float)80.0f)
#define WAVE_BANNER_FONT_SPACING ((float)8.0f)
#define WAVE_BANNER_COLOR ((Color){ 245, 245, 245, 255 })

#define ENEMY_FORMATION_ROWS 3
#define ENEMY_FORMATION_COLS 7
#define ENEMY_FORMATION_SPACING ((Vector2){ 60.0f, 28.0f })
#define ENEMY_FORMATION_INITIAL_POS ((Vector2){ (float)WINDOW_WIDTH * 0.5, (float)WINDOW_HEIGHT * -0.35 })
#define ENEMY_FORMATION_MAIN_Y ((float)WINDOW_HEIGHT * 0.18)
#define ENEMY_FORMATION_STRAFE_VEL_X ((float)220.0f)
#define ENEMY_FORMATION_ENTER_LEVEL_SPEED ((float)300)
#define ENEMY_FORMATION_STRAFE_PADDING ((float)15.0)

#define INVADER_HEALTH 1
#define INVADER_TOTAL_SHOOTING_TIME_LONG ((float)0.11f)
#define INVADER_TOTAL_SHOOTING_TIME_SHORT ((float)0.5f)
#define INVADER_WAIT_AFTER_SHOOTING_TIME_LONG ((float)2.4f)
#define INVADER_WAIT_AFTER_SHOOTING_TIME_SHORT ((float)1.5f)
#define INVADER_SPRITE_SCALE ((float)3.0f)

#define INVADER_BOUNDS_SIZE ((Vector2){ 85, 90 })

#define INVADER_MISSILE_COOLDOWN ((float)0.3f)
#define INVADER_MISSILE_SIZE ((Vector2){ 16, 30 })
#define INVADER_MISSILE_COLOR ((Color){ 0, 216, 70, 255 })
#define INVADER_MISSILE_SPAWN_OFFSET ((Vector2){ 0, 0.5f*(INVADER_MISSILE_SIZE.y + INVADER_BOUNDS_SIZE.y) })
#define INVADER_MISSILE_VELOCITY ((Vector2){ 0, 820 })
#define INVADER_MISSILE_COLLISION_MASK ((Entity_kind_mask)(ENTITY_KIND_MASK_PLAYER | 0))
#define INVADER_MISSILE_DAMAGE 1
#define INVADER_MISSILE_SPRITE_SCALE ((float)3.0f)

#define HEALTH_PACK_SIZE ((Vector2){ 50, 50 })
#define HEALTH_PACK_INITIAL_Y ((float)-80.0f)
#define HEALTH_PACK_VELOCITY ((Vector2){ 0, 200 })
#define HEALTH_PACK_SPRITE_SCALE ((float)5.0f)

#define SHIELDS_PICKUP_SIZE ((Vector2){ 60, 60 })
#define SHIELDS_PICKUP_INITIAL_Y ((float)-80.0f)
#define SHIELDS_PICKUP_VELOCITY ((Vector2){ 0, 480 })
#define SHIELDS_PICKUP_SPRITE_SCALE ((float)2.0f)

#define SHIELDS_TIME ((float)7.0f)
#define SHIELDS_COLOR_ANIMATE_FREQ ((float)40.0f)

#define ORANGE_EXPLOSION_SCALE ((float)3.0f)

#define SCORE_LABEL_FONT_SIZE ((float)40.0f)

#define GAME_OVER_BANNER_FONT_SIZE ((float)100.0f)
#define GAME_OVER_BANNER_PRE_DELAY ((float)1.7f)

#define PAUSE_BANNER_FONT_SIZE ((float)100.0f)
#define PAUSE_BANNER_COLOR (Color){ 245, 245, 245, 255 }

#define RESUME_HINT_FONT_SIZE ((float)30.0f)
#define RESUME_HINT_COLOR (Color){ 245, 245, 245, 205 }
#define RESUME_HINT_BLINK_PERIOD ((float)0.66)

#define ORANGE_FIRE_PARTICLE_SPRITE_SCALE ((float)2.0f)
#define PURPLE_FIRE_PARTICLE_SPRITE_SCALE ((float)3.0f)


/* macros */

#define entity_kind_in_mask(kind, mask) (!!(mask & (1ull<<kind)))


/* tables */

#define GAME_STATES             \
  X(NONE)                       \
  X(TITLE_SCREEN)               \
  X(SPAWN_PLAYER)               \
  X(WAVE_TRANSITION)            \
  X(SPAWN_ENEMIES)              \
  X(MAIN_LOOP)                  \
  X(GAME_OVER)                  \
  X(DEBUG_SANDBOX)              \

#define GAME_DEBUG_FLAGS     \
  X(DEBUG_UI)                \
  X(HOT_RELOAD)              \
  X(PLAYER_INVINCIBLE)       \
  X(DRAW_ALL_ENTITY_BOUNDS)  \
  X(SANDBOX_LOADED)          \

#define GAME_FLAGS      \
  X(PAUSE)              \

#define ENTITY_KINDS    \
  X(PLAYER)             \
  X(INVADER)            \
  X(FORMATION)          \
  X(MISSILE)            \
  X(BANNER)             \
  X(HEALTH_PACK)        \
  X(SHIELDS_PICKUP)     \

#define ENTITY_ORDERS   \
  X(FIRST)              \
  X(LAST)               \

#define ENTITY_FLAGS                 \
  X(DYNAMICS)                        \
  X(APPLY_FRICTION)                  \
  X(APPLY_COLLISION)                 \
  X(RECEIVE_COLLISION)               \
  X(DIE_ON_APPLY_COLLISION)          \
  X(CLAMP_POS_TO_SCREEN)             \
  X(HAS_SHIELDS)                     \
  X(HAS_MISSILE_LAUNCHER)            \
  X(HAS_PARTICLE_EMITTER)            \
  X(HAS_SPRITE)                      \
  X(MOVING)                          \
  X(APPLY_COLLISION_DAMAGE)          \
  X(RECEIVE_COLLISION_DAMAGE)        \
  X(BLINK_TEXT)                      \
  X(DRAW_TEXT)                       \
  X(USE_DAMAGE_BLINK_TINT)           \
  X(DRAW_BOUNDS)                     \
  X(FILL_BOUNDS)                     \

#define ENTITY_CONTROLS            \
  X(PLAYER)                        \
  X(ENEMY_FORMATION_ENTER_LEVEL)   \
  X(ENEMY_FORMATION_MAIN)          \
  X(INVADER_IN_FORMATION)          \
  X(MISSILE)                       \
  X(HEALTH_PACK)                   \
  X(SHIELDS_PICKUP)                \

#define PARTICLE_FLAGS            \
  X(HAS_SPRITE)                   \
  X(DIE_WHEN_ANIM_FINISH)         \
  X(MULTIPLE_ANIM_CYCLES)         \


/* type definitions */

typedef struct Game Game;
typedef struct Player_input Player_input;
typedef struct Entity Entity;
typedef struct Missile_launcher Missile_launcher;
typedef struct Particle_emitter Particle_emitter;
typedef struct Particle Particle;
typedef u64 Particle_flags;
typedef u64 Game_flags;
typedef u64 Game_debug_flags;
typedef u64 Entity_flags;
typedef u64 Entity_kind_mask;

typedef enum Game_state {
#define X(state) GAME_STATE_##state,
  GAME_STATES
#undef X
    GAME_STATE_MAX,
} Game_state;

char *Game_state_strings[GAME_STATE_MAX] = {
#define X(state) #state,
  GAME_STATES
#undef X
};

typedef enum Game_flag_index {
  GAME_FLAG_INDEX_INVALID = -1,
#define X(flag) GAME_FLAG_INDEX_##flag,
  GAME_FLAGS
#undef X
    GAME_FLAG_INDEX_MAX,
} Game_flag_index;

STATIC_ASSERT(GAME_FLAG_INDEX_MAX < 64, number_of_game_flags_is_less_than_64);

#define X(flag) const Game_flags GAME_FLAG_##flag = (Game_flags)(1ull<<GAME_FLAG_INDEX_##flag);
GAME_FLAGS
#undef X

typedef enum Game_debug_flag_index {
  GAME_DEBUG_FLAG_INDEX_INVALID = -1,
#define X(flag) GAME_DEBUG_FLAG_INDEX_##flag,
  GAME_DEBUG_FLAGS
#undef X
    GAME_DEBUG_FLAG_INDEX_MAX,
} Game_debug_flag_index;

STATIC_ASSERT(GAME_DEBUG_FLAG_INDEX_MAX < 64, number_of_game_debug_flags_is_less_than_64);

#define X(flag) const Game_debug_flags GAME_DEBUG_FLAG_##flag = (Game_debug_flags)(1ull<<GAME_DEBUG_FLAG_INDEX_##flag);
GAME_DEBUG_FLAGS
#undef X

typedef enum Entity_kind {
  ENTITY_KIND_INVALID = 0,
#define X(kind) ENTITY_KIND_##kind,
  ENTITY_KINDS
#undef X
    ENTITY_KIND_MAX,
} Entity_kind;

#define X(kind) const Entity_kind_mask ENTITY_KIND_MASK_##kind = (Entity_kind_mask)(1ull<<ENTITY_KIND_##kind);
ENTITY_KINDS
#undef X

STATIC_ASSERT(ENTITY_KIND_MAX < 64, number_of_entity_kinds_is_less_than_64);

typedef enum Entity_order {
  ENTITY_ORDER_INVALID = -1,
#define X(order) ENTITY_ORDER_##order,
  ENTITY_ORDERS
#undef X
    ENTITY_ORDER_MAX,
} Entity_order;

typedef enum Entity_control {
  ENTITY_CONTROL_NONE = 0,
#define X(control) ENTITY_CONTROL_##control,
  ENTITY_CONTROLS
#undef X
    ENTITY_CONTROL_MAX
} Entity_control;

typedef enum Entity_flag_index {
  ENTITY_FLAG_INDEX_INVALID = -1,
#define X(flag) ENTITY_FLAG_INDEX_##flag,
  ENTITY_FLAGS
#undef X
    ENTITY_FLAG_INDEX_MAX,
} Entity_flag_index;

#define X(flag) const Entity_flags ENTITY_FLAG_##flag = (Entity_flags)(1ull<<ENTITY_FLAG_INDEX_##flag);
ENTITY_FLAGS
#undef X

STATIC_ASSERT(ENTITY_FLAG_INDEX_MAX < 64, number_of_entity_flags_is_less_than_64);

typedef enum Particle_flag_index {
  PARTICLE_FLAG_INDEX_INVALID = -1,
#define X(flag) PARTICLE_FLAG_INDEX_##flag,
  PARTICLE_FLAGS
#undef X
    PARTICLE_FLAG_INDEX_MAX,
} Particle_flag_index;

#define X(flag) const Particle_flags PARTICLE_FLAG_##flag = (Particle_flags)(1ull<<PARTICLE_FLAG_INDEX_##flag);
PARTICLE_FLAGS
#undef X

STATIC_ASSERT(PARTICLE_FLAG_INDEX_MAX < 64, number_of_particle_flags_is_less_than_64);


/* struct bodies */

struct Particle {
  u32 live;

  Particle_flags flags;

  Vector2 pos;
  Vector2 vel;
  Vector2 half_size;

  Sprite    sprite;
  Color     sprite_tint;
  float     sprite_scale;
};

struct Particle_emitter {
  int emit_count;

  Particle particle;
};

struct Missile_launcher {
  Entity_flags missile_entity_flags;

  Vector2 initial_pos;
  Vector2 spawn_offset;
  Vector2 initial_vel;
  Vector2 missile_size;

  Color   missile_color;

  Sprite sprite;
  Color  sprite_tint;
  float  sprite_scale;

  Sound  *missile_sound;

  Entity_kind_mask collision_mask;
  int              damage_amount;

  Particle_emitter particle_emitter;

  u32     shooting;
  float   cooldown_period;
  float   cooldown_timer;
};

struct Entity {
  b32 live;

  Entity_kind    kind;
  Entity_order   update_order;
  Entity_order   draw_order;
  Entity_control control;
  Entity_flags   flags;

  u64 genid;

  Vector2 accel;
  Vector2 vel;
  Vector2 pos;
  Vector2 half_size;

  Color bounds_color;
  Color fill_color;

  Vector2 formation_slot_size;
  u64     formation_id;

  float formation_trigger_enemy_shoot_delay;

  Missile_launcher missile_launcher;

  Entity_kind_mask collision_mask;

  int              damage_amount;

  Sound missile_sound;

  float enemy_total_shooting_time;
  float enemy_wait_after_shooting_time;
  b32   enemy_do_spit_animation;

  int health;
  int received_damage;

  float shields_time;

  Particle_emitter particle_emitter;

  Sprite sprite;
  Color     sprite_tint;
  float     sprite_scale;

  float damage_blink_timer;
  float damage_blink_period;
  float damage_blink_total_time;
  int   damage_blink_high;
  Color damage_blink_sprite_tint;

  char *text;
  float blink_timer;
  float blink_period;
  float font_size;
  float font_spacing;
  Color text_color;

  Entity *free_list_next;
};

struct Player_input {
  bool move_left;
  bool move_right;
  bool stop_move_left;
  bool stop_move_right;
  bool shoot;
  bool pressed_any_key;
  bool restarted_game;
  bool hot_reload;
};

struct Game {
  float timestep;

  Game_state state;
  Game_state next_state;

  Game_flags       flags;
  Game_debug_flags debug_flags;

  u64 frame_index;

  Arena frame_scratch;

  char wave_banner_buf[256];

  Entity  entities[MAX_ENTITIES];
  s64     entities_allocated;
  Entity *entity_free_list;

  s64     save_entities_allocated;
  Entity *save_entity_free_list;

  Particle particles[MAX_PARTICLES];
  s64      particles_buf_pos;

  s64 live_entities;
  s64 live_particles;

  s64 live_missiles;

  s64 score;

  Player_input player_input;

  Entity *title_screen_banner;
  Entity *title_screen_hint;
  Entity *wave_banner;
  Entity *player;

  u64 player_genid;

  Entity *enemy_formation;

  u64 current_formation_id;

  Font font;

  RenderTexture2D render_texture;

  int wave_counter;

  float     background_y_offset;
  float     background_scroll_speed;
  Texture2D background_texture;
  Texture2D sprite_atlas;

  Sound invader_missile_sound;
  Sound player_missile_sound;
  Sound invader_die_sound;
  Sound player_damage_sound;
  Sound player_die_sound;
  Sound wave_banner_sound;
  Sound hyperspace_jump_sound;

  float time_since_last_health_spawned;
  float time_since_last_shields_spawned;

  float wave_transition_pre_delay_timer;
  float wave_transition_post_delay_timer;
  float wave_transition_ramp_timer;
  float wave_transition_banner_timer;

  float game_over_banner_pre_delay_timer;

  float resume_hint_blink_timer;
  int   resume_hint_blink_high;

  bool title_screen_scroll_title;
  bool wave_start_ramped_background_scroll_speed;
  bool spawned_enemies;
  bool show_game_over_banner;
  bool spawned_player;

};


/* function headers */

void  game_update_and_draw(Game *gp);
void  game_reset_frame_scratch(Game *gp);
void* game_frame_scratch_alloc(Game *gp, size_t bytes);
void  game_reset(Game *gp);

Entity *entity_spawn(Game *gp);
void    entity_die(Game *gp, Entity *ep);

void entity_init_title_banner(Game *gp, Entity *ep);
void entity_init_title_screen_hint_text(Game *gp, Entity *ep);
void entity_init_player(Game *gp, Entity *ep);
void entity_init_wave_banner(Game *gp, Entity *ep);
void entity_init_enemy_formation(Game *gp, Entity *ep);
void entity_init_invader_in_formation(Game *gp, Entity *ep, u64 formation_id, Vector2 initial_pos);
void entity_init_missile_from_launcher(Game *gp, Entity *ep, Missile_launcher launcher);
void entity_init_health_pack(Game *gp, Entity *ep);
void entity_init_shields_pickup(Game *gp, Entity *ep);

void         sprite_update(Game *gp, Sprite *sp);
Sprite_frame sprite_current_frame(Sprite sp);
b32          sprite_at_keyframe(Sprite sp, s32 keyframe);
void         draw_sprite(Game *gp, Sprite sp, Vector2 pos, Color tint);
void         draw_sprite_ex(Game *gp, Sprite sp, Vector2 pos, f32 scale, f32 rotation, Color tint);

void particle_emit(Game *gp, Particle_emitter emitter);

#endif
