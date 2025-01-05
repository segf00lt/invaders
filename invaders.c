#include <raylib.h>
#include <raymath.h>
#include "basic.h"
#include "stb_sprintf.h"


typedef struct Entity Entity;
typedef struct Missile Missile;
typedef struct Missile_emitter Missile_emitter;
typedef struct Enemy_formation Enemy_formation;


/* constants */

#define MAX_MISSILES (int)256
#define ENEMY_WAVE_SIZE (int)21
#define MAX_SIMULTANEOUS_FIRING_ENEMIES (int)5
#define MAX_ENEMIES (int)32

const float ASPECT = 1.0; //TODO adjustable aspect ratio
const int   SCREEN_WIDTH = 1100;
const int   SCREEN_HEIGHT = 900;

const int   SHIP_Y_COORD = SCREEN_HEIGHT - 200;
const float SHIP_ACCEL = 4e4;
const float SHIP_START_GAME_Y_VELOCITY = -200;
const int   SHIP_TOTAL_HEALTH = 5;
const float SHIP_FIRE_RATE = 0.3;

const int   STRAFE_X_MAX_OFFSET = 80;
const float ENEMY_START_GAME_Y_VELOCITY = 300;
const float ENEMY_STRAFE_SPEED = 100;
const float ENEMY_FORMATION_INITIAL_TOP_LEFT_Y = 150;
const float ENEMY_EVEN_FIRING_TIME = 2;
const float ENEMY_ODD_FIRING_TIME = 1;
const float ENEMY_EVEN_FIRE_RATE = 0.8;
const float ENEMY_ODD_FIRE_RATE = 0.3;



/* struct definitions */

struct Entity {
    Vector2   pos;
    Vector2   accel;
    Vector2   vel;
    float     half_width;
    float     half_height;

    Texture2D sprite;
    int       missile_emitter_id;

    float     fire_rate;
    float     shot_timer;
    float     total_firing_time;

    int       health;

    bool      hit;
    bool      live;
};

struct Missile {
    Vector2   pos;
    Vector2   vel;
    float     half_width;
    float     half_height;
    Rectangle rec;
    Color     color;
};

struct Missile_emitter {
    Missile buf[MAX_MISSILES];
    bool    live[MAX_MISSILES];
    Entity *missile_targets;
    int     missile_targets_count;
    Color   color;
    int     n;
};

struct Enemy_formation {
    int     enemy_indexes[ENEMY_WAVE_SIZE];
    int     firing_enemy_indexes[MAX_SIMULTANEOUS_FIRING_ENEMIES];
    int     enemy_count;
    int     firing_enemy_count;
    Vector2 strafe_vel;
};


/* function headers */

int  make_enemy(Vector2 pos);
void destroy_enemy(int i);
int  make_missile(Missile_emitter *emitter, Vector2 pos, Vector2 vel);
void destroy_missile(Missile_emitter *emitter, int i);
void main_game_update(void);
void start_game_update(void);
void wave_transition_update(void); //TODO
void draw(void);
int  main(void);


/* globals */

float timestep;
bool game_over = false;

bool ship_is_in_position = false;

Texture2D enemy_sprite;

Texture2D background;
Rectangle background_frame;
float background_scroll_speed = 400;

Missile_emitter missile_emitter_buf[MAX_ENEMIES + 1];
int missile_emitter_buf_allocated = 0;

Entity enemy_buf[MAX_ENEMIES];
int live_enemies_count = ENEMY_WAVE_SIZE;
int enemy_buf_allocated = 0;
Vector2 enemy_formation_top_left_pos;
Enemy_formation enemy_formation = { .strafe_vel = { -ENEMY_STRAFE_SPEED } };

void (*game_update)(void);

Entity ship;

Camera2D camera = { .zoom = ASPECT };


/* functions */

INLINE int make_enemy(Vector2 pos) {
    int i = 0;
    int increment = 1;
    for(; i < enemy_buf_allocated; ++i) {
        if(enemy_buf[i].live == false) {
            increment = 0;
            break;
        }
    }

    Entity *e = enemy_buf + i;
    e->pos.x = pos.x;
    e->pos.y = pos.y;
    e->half_width = enemy_sprite.width*0.5;
    e->half_height = enemy_sprite.height*0.5;
    e->health = 1;

    e->missile_emitter_id = missile_emitter_buf_allocated;
    missile_emitter_buf_allocated += 1;

    missile_emitter_buf[e->missile_emitter_id].color = (Color){ 0, 255, 100, 255 };

    missile_emitter_buf[e->missile_emitter_id].missile_targets = &ship;
    missile_emitter_buf[e->missile_emitter_id].missile_targets_count = 1;

    e->sprite = enemy_sprite;

    enemy_buf[i].live = true;
    enemy_buf_allocated += increment;

    return i;
}

INLINE void destroy_enemy(int i) {
    enemy_buf[i].live = false;
}

INLINE int make_missile(Missile_emitter *emitter, Vector2 pos, Vector2 vel) {
    int i = 0;
    int increment = 1;
    for(; i < emitter->n; ++i) {
        if(emitter->live[i] == false) {
            increment = 0;
            break;
        }
    }

    Missile *m = emitter->buf + i;
    m->rec = (Rectangle){ .width = 5.5, .height = 14 };
    m->color = emitter->color;
    m->pos.x = pos.x;
    m->pos.y = pos.y;
    m->half_width = m->rec.width*0.5;
    m->half_height = m->rec.height*0.5;
    m->vel = vel;

    emitter->live[i] = true;
    emitter->n += increment;

    return i;
}

INLINE void destroy_missile(Missile_emitter *emitter, int i) {
    emitter->live[i] = false;
}

void main_game_update(void) {
    background_frame.y -= background_scroll_speed*timestep;
    if(background_frame.y <= -SCREEN_HEIGHT)
        background_frame.y = background.height - SCREEN_HEIGHT;

    { /* ship update */

        if(ship.hit == true) {
            ship.health -= 1;
            if(ship.health <= 0) {
                ship.live = false;
            }
            ship.hit = false;
        }

        if(ship.live == false) {
            printf("you lose\n");
            game_over = true;
        }

        ship.accel.x = 0;

        if(IsKeyDown(KEY_A)) ship.accel.x += -SHIP_ACCEL;
        else ship.vel.x = 0;

        if(IsKeyDown(KEY_D)) ship.accel.x += SHIP_ACCEL;
        else ship.vel.x = 0;

        float a_times_t = ship.accel.x * timestep;
        ship.vel.x += a_times_t;
        ship.pos.x += a_times_t*timestep*0.5 + ship.vel.x*timestep;

        ship.pos.x = Clamp(ship.pos.x, ship.half_width, SCREEN_WIDTH - ship.half_width);

        if(IsKeyDown(KEY_J)) {

            if(ship.shot_timer <= 0) {
                ship.shot_timer = ship.fire_rate;
                make_missile(
                        missile_emitter_buf + ship.missile_emitter_id,
                        (Vector2){ .x = ship.pos.x, .y = ship.pos.y - ship.half_height - 2.0},
                        (Vector2){ 0, -1000 }
                        );
            } else {
                ship.shot_timer -= timestep;
            }
        }

    } /* ship update */

    { /* enemy update */

        live_enemies_count = 0;
        for(int i = 0; i < enemy_buf_allocated; ++i) {
            live_enemies_count += enemy_buf[i].live;
        }

        for(int i = 0; i < enemy_formation.firing_enemy_count; ++i) {
            if(enemy_buf[enemy_formation.firing_enemy_indexes[i]].live && enemy_buf[enemy_formation.firing_enemy_indexes[i]].total_firing_time > 0.0) {
                continue;
            }

            int enemy_index = GetRandomValue(0, enemy_buf_allocated);
            while(live_enemies_count > 0 && enemy_buf[enemy_index].live == false) {
                enemy_index = GetRandomValue(0, enemy_buf_allocated);
            }

            if(enemy_index & 0x1) {
                enemy_buf[enemy_index].total_firing_time = ENEMY_ODD_FIRING_TIME;
                enemy_buf[enemy_index].fire_rate = ENEMY_ODD_FIRE_RATE;
            } else {
                enemy_buf[enemy_index].total_firing_time = ENEMY_EVEN_FIRING_TIME;
                enemy_buf[enemy_index].fire_rate = ENEMY_EVEN_FIRE_RATE;
            }
            enemy_formation.firing_enemy_indexes[i] = enemy_index;

        }

        bool swap_strafe_direction = false;

        for(int i = 0; i < enemy_formation.enemy_count; ++i) {
            int enemy_index = enemy_formation.enemy_indexes[i];
            if(enemy_buf[enemy_index].live == false) {
                enemy_formation.enemy_indexes[i] = enemy_formation.enemy_indexes[enemy_formation.enemy_count-1];
                enemy_formation.enemy_count -= 1;
            }

            Entity *e = enemy_buf + enemy_index;
            e->vel = enemy_formation.strafe_vel;

            if(enemy_formation.strafe_vel.x < 0.0 && e->pos.x <= STRAFE_X_MAX_OFFSET) {
                swap_strafe_direction = true;
            } else if(enemy_formation.strafe_vel.x > 0 && e->pos.x > SCREEN_WIDTH - STRAFE_X_MAX_OFFSET) {
                swap_strafe_direction = true;
            }
        }

        if(swap_strafe_direction) {
            swap_strafe_direction = false;
            enemy_formation.strafe_vel = Vector2Negate(enemy_formation.strafe_vel);
        }

        for(int i = 0; i < enemy_buf_allocated; ++i) {
            if(enemy_buf[i].hit == true) {
                enemy_buf[i].health -= 1;
                if(enemy_buf[i].health <= 0)
                    enemy_buf[i].live = false;
                enemy_buf[i].hit = false;
            }

            if(enemy_buf[i].live == false) continue;

            Entity *e = enemy_buf + i;
            e->pos = Vector2Add(e->pos, Vector2Scale(e->vel, timestep));
            Missile_emitter *enemy_missile_emitter = missile_emitter_buf + e->missile_emitter_id;

            if(e->total_firing_time > 0.0) {
                if(e->shot_timer <= 0) {
                    e->shot_timer = e->fire_rate;
                    make_missile(
                            enemy_missile_emitter,
                            (Vector2){ .x = e->pos.x, .y = e->pos.y + e->half_height + 2.0},
                            (Vector2){ 0, 700 }
                            );
                }

                e->shot_timer -= timestep;

                e->total_firing_time -= timestep;
            } else {
                e->total_firing_time = 0.0;
            }

        }

    } /* enemy update */

    { /* missile emitter update */
        Missile_emitter *last_emitter = missile_emitter_buf + STATICARRLEN(missile_emitter_buf);
        for(Missile_emitter *missile_emitter = missile_emitter_buf;  missile_emitter < last_emitter; ++missile_emitter) {
            int n_consecutive_dead_missiles = 0;
            for(int i = 0; i < missile_emitter->n; ++i) {
                if(missile_emitter->live[i] == false) {
                    n_consecutive_dead_missiles += 1;
                    continue;
                }

                n_consecutive_dead_missiles = 0;

                Missile *m = missile_emitter->buf + i;
                m->pos.y += m->vel.y*timestep;

                for(int j = 0; j < missile_emitter->missile_targets_count; ++j) {
                    Entity *target_entity = missile_emitter->missile_targets + j;

                    if(target_entity->live == false) continue;

                    Rectangle target_hitbox =
                    {
                        .x = target_entity->pos.x - target_entity->half_width,
                        .y = target_entity->pos.y - target_entity->half_height,
                        .width = TIMES2(target_entity->half_width),
                        .height = TIMES2(target_entity->half_height),
                    };

                    Rectangle missile_hitbox =
                    {
                        .x = m->pos.x - m->half_width, .y = m->pos.y - m->half_height,
                        .width = m->rec.width, .height = m->rec.height,
                    };

                    if(CheckCollisionRecs(target_hitbox, missile_hitbox)) {
                        destroy_missile(missile_emitter, i);
                        target_entity->hit = true;
                    }
                }

            }

            missile_emitter->n -= n_consecutive_dead_missiles;

        }

    } /* missile_emitter update */
}

void start_game_update(void) {
    background_frame.y -= background_scroll_speed*timestep;
    if(background_frame.y <= -SCREEN_HEIGHT)
        background_frame.y = background.height - SCREEN_HEIGHT;

    if(!ship_is_in_position) { /* ship update */
        ship.pos.y += SHIP_START_GAME_Y_VELOCITY * timestep;
        if(ship.pos.y <= SHIP_Y_COORD) {
            ship.pos.y = SHIP_Y_COORD;
            ship_is_in_position = true;
        }
    }

    if(ship_is_in_position) { /* enemy update */

        enemy_formation_top_left_pos.y += ENEMY_START_GAME_Y_VELOCITY * timestep;

        for(int i = 0; i < enemy_formation.enemy_count; ++i) {
            int enemy_index = enemy_formation.enemy_indexes[i];

            assert(enemy_buf[enemy_index].live);

            Entity *e = enemy_buf + enemy_index;
            e->pos.y += ENEMY_START_GAME_Y_VELOCITY * timestep;
        }

        if(enemy_formation_top_left_pos.y >= ENEMY_FORMATION_INITIAL_TOP_LEFT_Y) {
            enemy_formation_top_left_pos.y = ENEMY_FORMATION_INITIAL_TOP_LEFT_Y;
            game_update = main_game_update;
        }
    }
}

void wave_transition_update(void) {
}

INLINE void draw(void) {
#ifdef DEBUG
    char buf[128];
#endif

    BeginDrawing();
    ClearBackground(BLACK);
    BeginMode2D(camera);

    DrawTextureRec(background, background_frame, (Vector2){0}, (Color){255,255,255,140});

    DrawTextureV(ship.sprite,
            (Vector2){ .x = ship.pos.x - ship.sprite.width*0.5, .y = ship.pos.y - ship.sprite.height*0.5 },
            WHITE);
#ifdef DEBUG
    Rectangle hitbox =
    {
        .x = ship.pos.x - ship.half_width, .y = ship.pos.y - ship.half_height,
        .width = TIMES2(ship.half_width),
        .height = TIMES2(ship.half_height),
    };
    DrawRectangleLinesEx(hitbox, 1.0, YELLOW);
#endif

    for(int i = 0; i < enemy_buf_allocated; ++i) {
        if(enemy_buf[i].live == false) continue;

        Entity *e = enemy_buf + i;

        DrawTextureV(e->sprite,
                (Vector2){
                .x = e->pos.x - e->sprite.width*0.5,
                .y = e->pos.y - e->sprite.height*0.5
                },
                WHITE);

#ifdef DEBUG
        Rectangle hitbox =
        {
            .x = ship.pos.x - ship.half_width, .y = ship.pos.y - ship.half_height,
            .width = TIMES2(ship.half_width),
            .height = TIMES2(ship.half_height),
        };
        DrawRectangleLinesEx(hitbox, 1.0, YELLOW);
#endif
    }

    for(int i = 0; i < missile_emitter_buf_allocated; ++i) {
        Missile_emitter *missile_emitter = missile_emitter_buf + i;

        for(int i = 0; i < missile_emitter->n; ++i) {
            if(missile_emitter->live[i] == false) continue;

            Missile *m = missile_emitter->buf + i;
            Rectangle rec =
            {
                .x = m->pos.x - m->half_width, .y = m->pos.y - m->half_height,
                .width = m->rec.width, .height = m->rec.height,
            };
            DrawRectangleRec(rec, m->color);
        }
    }

    EndMode2D();

    {
        float scale = 0.7;
        float x = 8;
        float y = SCREEN_HEIGHT - ship.sprite.height*scale - 20;
        float x_step = ship.sprite.width * scale - 20;

        for(int i = 0; i < ship.health; ++i) {
            DrawTextureEx(ship.sprite, (Vector2){ x , y }, 0.0, scale, WHITE);
            x += x_step;
        }
    }

#ifdef DEBUG
    stbsp_sprintf(buf, "FPS: %i, FRAMETIME: %f\n", GetFPS(), timestep);
    DrawText(buf, 20, 20, 20, RAYWHITE);
#endif

    EndDrawing();
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "invaders");
    SetTargetFPS(120);

    //SetRandomSeed(42);

    ship.sprite = LoadTexture("sprites/ship.png");
    enemy_sprite = LoadTexture("sprites/enemy.png");
    background = LoadTexture("sprites/nightsky.png");

    background_frame = (Rectangle){
        .y = background.height - SCREEN_HEIGHT,
        .width = SCREEN_WIDTH,
        .height = SCREEN_HEIGHT
    };

    { /* initialize enemies */

        int row_size = ENEMY_WAVE_SIZE / 3;
        float formation_spacing = 50.0;
        float formation_width = (enemy_sprite.width + formation_spacing) * row_size;
        float formation_x_offset = (SCREEN_WIDTH - formation_width) * 0.5;

        float width_section = formation_width / row_size;
        float half_width_section = width_section*0.5;
        float current_x = formation_x_offset;
        float current_y = ENEMY_FORMATION_INITIAL_TOP_LEFT_Y - 800;
        for(int i = 0; i < ENEMY_WAVE_SIZE;) {
            int enemy_index = make_enemy((Vector2){ current_x + half_width_section, current_y });
            current_x += width_section;

            enemy_formation.enemy_indexes[enemy_formation.enemy_count++] = enemy_index;

            i++;

            if(i % row_size == 0) {
                current_y += enemy_sprite.height*0.5 + formation_spacing;
                current_x = formation_x_offset;
            }
        }

        for(int i = 0; i < MAX_SIMULTANEOUS_FIRING_ENEMIES; ++i) {
            int enemy_index = GetRandomValue(0, enemy_buf_allocated - 1);
            if(enemy_index & 0x1) {
                enemy_buf[enemy_index].total_firing_time = ENEMY_ODD_FIRING_TIME;
                enemy_buf[enemy_index].fire_rate = ENEMY_ODD_FIRE_RATE;
            } else {
                enemy_buf[enemy_index].total_firing_time = ENEMY_EVEN_FIRING_TIME;
                enemy_buf[enemy_index].fire_rate = ENEMY_EVEN_FIRE_RATE;
            }
            enemy_buf[enemy_index].shot_timer = 0.0;
            enemy_formation.firing_enemy_indexes[enemy_formation.firing_enemy_count++] = enemy_index;
        }

        enemy_formation_top_left_pos =
            (Vector2) {
                .x = enemy_buf[enemy_formation.enemy_indexes[0]].pos.x,
                .y = enemy_buf[enemy_formation.enemy_indexes[0]].pos.y,
            };

    } /* initialize enemies */

    { /* initialize ship */
        ship.missile_emitter_id = missile_emitter_buf_allocated;
        missile_emitter_buf_allocated += 1;
        Missile_emitter *missile_emitter = &missile_emitter_buf[ship.missile_emitter_id];
        missile_emitter->color = (Color){ 255, 10, 40, 255 };
        missile_emitter->missile_targets = enemy_buf;
        missile_emitter->missile_targets_count = live_enemies_count;
        ship.pos.x = HALF(SCREEN_WIDTH);
        ship.pos.y = SHIP_Y_COORD + 400;
        ship.half_width = 20;
        ship.half_height = 30;
        ship.live = true;
        ship.health = SHIP_TOTAL_HEALTH;
        ship.fire_rate = SHIP_FIRE_RATE;
    } /* initialize ship */

    game_update = start_game_update;

    game_over = game_over + 0;

    while(!WindowShouldClose() && !game_over && live_enemies_count > 0) {
        timestep = GetFrameTime();

        //__builtin_dump_struct(&enemy_formation, printf);

        game_update();

        draw();

    }

    UnloadTexture(ship.sprite);
    UnloadTexture(enemy_sprite);
    UnloadTexture(background);

    CloseWindow();

    return 0;
}
