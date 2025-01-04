#include <raylib.h>
#include <raymath.h>
#include "basic.h"
#include "pool.h"
#include "stb_sprintf.h"

const float ASPECT = 1.0; //TODO adjustable aspect ratio
const int SCREEN_WIDTH = 1100;
const int SCREEN_HEIGHT = 900;
const int SHIP_Y_COORD = SCREEN_HEIGHT - 200;
const float SHIP_ACCEL = 4e4;
const int MAX_MISSILES = 256;
const int ENEMY_WAVE_SIZE = 14;

const int STRAFE_X_MAX_OFFSET = 80;
const float ENEMY_STRAFE_SPEED = 100;
const int MAX_SIMULTANEOUS_FIRING_ENEMIES = 4;

Texture2D enemy_sprite;
Texture2D background;

struct Entity {
    Vector2   pos;
    Vector2   accel;
    Vector2   vel;
    float     half_width;
    float     half_height;
};

struct Missile : Entity {
    Rectangle rec = { .width = 5, .height = 14, };
    Color     color = { 255, 50, 0, 255 };
};

struct Missile_emitter {
    Missile buf[MAX_MISSILES];
    bool live[MAX_MISSILES];
    int pos = 0;
};

struct Enemy : Entity {
    Texture2D       sprite;
    Missile_emitter missile_launcher;
    float           fire_rate;
    float           time_since_last_shot;
    float           total_firing_time;
};

struct Enemy_formation {
    int enemy_indexes[ENEMY_WAVE_SIZE];
    int firing_enemy_indexes[MAX_SIMULTANEOUS_FIRING_ENEMIES];
    int enemy_count;
    int firing_enemy_count;
    Vector2 strafe_vel;
};

struct Ship : Entity {
    Texture2D       sprite;
    Missile_emitter missile_launcher;
};

const int MAX_ENEMIES = 32;
Enemy enemy_buf[MAX_ENEMIES];
bool live_enemies[MAX_ENEMIES];
int enemy_buf_pos = 0;

Ship ship;

INLINE int make_enemy(Vector2 pos) {
    int i = 0;
    int increment = 1;
    for(; i < enemy_buf_pos; ++i) {
        if(live_enemies[i] == false) {
            increment = 0;
            break;
        }
    }

    Enemy *e = enemy_buf + i;
    Enemy init;
    init.pos.x = pos.x;
    init.pos.y = pos.y;
    init.half_width = enemy_sprite.width*0.5;
    init.half_height = enemy_sprite.height*0.5;
    *e = init;

    e->sprite = enemy_sprite;

    live_enemies[i] = true;
    enemy_buf_pos += increment;

    return i;
}

INLINE void destroy_Enemy(int i) {
    live_enemies[i] = false;
}

INLINE int make_missile(Missile_emitter *emitter, Vector2 pos, Vector2 vel) {
    int i = 0;
    int increment = 1;
    for(; i < emitter->pos; ++i) {
        if(emitter->live[i] == false) {
            increment = 0;
            break;
        }
    }

    Missile *m = emitter->buf + i;
    Missile init;
    init.pos.x = pos.x;
    init.pos.y = pos.y;
    init.half_width = init.rec.width*0.5;
    init.half_height = init.rec.height*0.5;
    *m = init;
    m->vel = vel;

    emitter->live[i] = true;
    emitter->pos += increment;

    return i;
}

INLINE void destroy_missile(Missile_emitter *emitter, int i) {
    emitter->live[i] = false;
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "invaders");
    SetTargetFPS(120);

    SetRandomSeed(42);

    ship.sprite = LoadTexture("sprites/ship.png");
    enemy_sprite = LoadTexture("sprites/enemy.png");
    background = LoadTexture("sprites/nightsky.png");

    Rectangle background_frame = {
        .y = background.height - SCREEN_HEIGHT,
        .width = SCREEN_WIDTH,
        .height = SCREEN_HEIGHT
    };

    float background_scroll_speed = 400;

    Camera2D camera = { .zoom = ASPECT };

    {
        Entity *e = &ship;
        e->pos.x = HALF(SCREEN_WIDTH);
        e->pos.y = SHIP_Y_COORD;
        e->half_width = 28;
        e->half_height = 38;
    }

    Enemy_formation enemy_formation = { .strafe_vel = { -ENEMY_STRAFE_SPEED } };
    
    {
        int row_size = ENEMY_WAVE_SIZE >> 1;
        float formation_spacing = 50.0;
        float formation_width = (enemy_sprite.width + formation_spacing) * row_size;
        float formation_x_offset = (SCREEN_WIDTH - formation_width) * 0.5;

        float width_section = formation_width / row_size;
        float half_width_section = width_section*0.5;
        float current_x = formation_x_offset;
        float current_y = 150;
        for(int i = 0; i < ENEMY_WAVE_SIZE;) {
            int enemy_index = make_enemy({ current_x + half_width_section, current_y });
            current_x += width_section;

            enemy_formation.enemy_indexes[enemy_formation.enemy_count++] = enemy_index;

            i++;

            if(i % row_size == 0) {
                current_y += enemy_sprite.height*0.5 + formation_spacing;
                current_x = formation_x_offset;
            }
        }

        float even_firing_time = 2;
        float odd_firing_time = 1;
        float even_fire_rate = 0.8;
        float odd_fire_rate = 0.3;
        for(int i = 0; i < MAX_SIMULTANEOUS_FIRING_ENEMIES; ++i) {
            int enemy_index = GetRandomValue(0, enemy_buf_pos);
            if(enemy_index & 0x1) {
                enemy_buf[enemy_index].total_firing_time = odd_firing_time;
                enemy_buf[enemy_index].fire_rate = odd_fire_rate;
            } else {
                enemy_buf[enemy_index].total_firing_time = even_firing_time;
                enemy_buf[enemy_index].fire_rate = even_fire_rate;
            }
            enemy_formation.firing_enemy_indexes[enemy_formation.firing_enemy_count++] = enemy_index;
        }
    }

    char buf[128];
    bool game_over = false;

    while(!WindowShouldClose() && !game_over) {
        float timestep = GetFrameTime();

        /* update */ {
            background_frame.y -= background_scroll_speed*timestep;
            if(background_frame.y <= -SCREEN_HEIGHT)
                background_frame.y = background.height - SCREEN_HEIGHT;

            Entity *e = &ship;

            e->accel.x = 0;

            if(IsKeyDown(KEY_A)) e->accel.x += -SHIP_ACCEL;
            else e->vel.x = 0;

            if(IsKeyDown(KEY_D)) e->accel.x += SHIP_ACCEL;
            else e->vel.x = 0;

            float a_times_t = e->accel.x * timestep;
            e->vel.x += a_times_t;
            e->pos.x += a_times_t*timestep*0.5 + e->vel.x*timestep;

            e->pos.x = Clamp(e->pos.x, e->half_width, SCREEN_WIDTH - e->half_width);

            if(IsKeyPressed(KEY_J)) {
                make_missile(
                        &ship.missile_launcher,
                        { .x = ship.pos.x, .y = ship.pos.y - ship.half_height - 2.0},
                        (Vector2){ 0, -1000 }
                        );
            }

            int n_consecutive_dead_missiles = 0;
            for(int i = 0; i < ship.missile_launcher.pos; ++i) {
                if(ship.missile_launcher.live[i] == false) {
                    n_consecutive_dead_missiles += 1;
                    continue;
                }

                n_consecutive_dead_missiles = 0;

                Missile *m = ship.missile_launcher.buf + i;
                m->pos.y += m->vel.y*timestep;

                bool hit = false;
                for(int j = 0; j < enemy_buf_pos; ++j) {
                    if(live_enemies[j] == false) {
                        continue;
                    }

                    Enemy *e = enemy_buf + j;

                    Rectangle enemy_hitbox =
                    {
                        .x = e->pos.x - e->half_width, .y = e->pos.y - e->half_height,
                        .width = TIMES2(e->half_width),
                        .height = TIMES2(e->half_height),
                    };

                    Rectangle missile_hitbox =
                    {
                        .x = m->pos.x - m->half_width, .y = m->pos.y - m->half_height,
                        .width = m->rec.width, .height = m->rec.height,
                    };

                    if(CheckCollisionRecs(enemy_hitbox, missile_hitbox)) {
                        destroy_missile(&ship.missile_launcher, i);
                        live_enemies[j] = false;
                    }
                }

                if(!hit && m->pos.y < -m->rec.height) {
                    destroy_missile(&ship.missile_launcher, i);
                }
            }

            ship.missile_launcher.pos -= n_consecutive_dead_missiles;

            /* enemy formation update */ {

                float even_firing_time = 2;
                float odd_firing_time = 1;
                float even_fire_rate = 0.8;
                float odd_fire_rate = 0.3;
                for(int i = 0; i < enemy_formation.firing_enemy_count; ++i) {
                    if(enemy_buf[enemy_formation.firing_enemy_indexes[i]].total_firing_time > 0.0) {
                        continue;
                    }

                    int enemy_index = GetRandomValue(0, enemy_buf_pos);
                    while(live_enemies[enemy_index] == false)
                        enemy_index = GetRandomValue(0, enemy_formation.enemy_count);

                    if(enemy_index & 0x1) {
                        enemy_buf[enemy_index].total_firing_time = odd_firing_time;
                        enemy_buf[enemy_index].fire_rate = odd_fire_rate;
                    } else {
                        enemy_buf[enemy_index].total_firing_time = even_firing_time;
                        enemy_buf[enemy_index].fire_rate = even_fire_rate;
                    }
                    enemy_formation.firing_enemy_indexes[i] = enemy_index;

                }

                bool swap_strafe_direction = false;

                for(int i = 0; i < enemy_formation.enemy_count; ++i) {
                    int enemy_index = enemy_formation.enemy_indexes[i];
                    if(live_enemies[enemy_index] == false) {
                        enemy_formation.enemy_indexes[i] = enemy_formation.enemy_indexes[enemy_formation.enemy_count-1];
                        enemy_formation.enemy_count -= 1;
                    }

                    Enemy *e = enemy_buf + enemy_index;
                    e->vel = enemy_formation.strafe_vel;

                    if(enemy_formation.strafe_vel.x < 0.0 && e->pos.x <= STRAFE_X_MAX_OFFSET) {
                        swap_strafe_direction = true;
                        printf("here\n");
                    } else if(enemy_formation.strafe_vel.x > 0 && e->pos.x > SCREEN_WIDTH - STRAFE_X_MAX_OFFSET) {
                        swap_strafe_direction = true;
                    }
                }

                if(swap_strafe_direction) {
                    swap_strafe_direction = false;
                    enemy_formation.strafe_vel = Vector2Negate(enemy_formation.strafe_vel);
                }

                for(int i = 0; i < enemy_buf_pos; ++i) {
                    if(live_enemies[i] == false) continue;
                    Enemy *e = enemy_buf + i;
                    e->pos += e->vel * timestep;

                    if(e->total_firing_time > 0.0) {
                        if(e->time_since_last_shot >= e->fire_rate) {
                            make_missile(
                                    &e->missile_launcher,
                                    { .x = e->pos.x, .y = e->pos.y + e->half_height + 2.0},
                                    (Vector2){ 0, 900 }
                                    );
                            e->time_since_last_shot = 0;
                        }

                        e->time_since_last_shot += timestep;

                        e->total_firing_time -= timestep;
                    } else {
                        e->total_firing_time = 0.0;
                    }

                    int n_consecutive_dead_missiles = 0;
                    for(int i = 0; i < e->missile_launcher.pos; ++i) {
                        if(e->missile_launcher.live[i] == false) {
                            n_consecutive_dead_missiles += 1;
                            continue;
                        }

                        n_consecutive_dead_missiles = 0;

                        Missile *m = e->missile_launcher.buf + i;
                        m->pos.y += m->vel.y*timestep;

                        bool hit = false;
                        for(int j = 0; j < enemy_buf_pos; ++j) {
                            if(live_enemies[j] == false) {
                                continue;
                            }

                            Enemy *e = enemy_buf + j;

                            Rectangle ship_hitbox =
                            {
                                .x = ship.pos.x - ship.half_width, .y = ship.pos.y - ship.half_height,
                                .width = TIMES2(ship.half_width),
                                .height = TIMES2(ship.half_height),
                            };

                            Rectangle missile_hitbox =
                            {
                                .x = m->pos.x - m->half_width, .y = m->pos.y - m->half_height,
                                .width = m->rec.width, .height = m->rec.height,
                            };

                            if(CheckCollisionRecs(ship_hitbox, missile_hitbox)) {
                                destroy_missile(&e->missile_launcher, i);
                                game_over = true;
                                printf("hit player\n");
                            }
                        }

                        if(!hit && m->pos.y < -m->rec.height) {
                            destroy_missile(&e->missile_launcher, i);
                        }
                    }

                    e->missile_launcher.pos -= n_consecutive_dead_missiles;
                }

            }
        }


        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(camera);


        /* draw */ {

            DrawTextureRec(background, background_frame, (Vector2){0}, (Color){255,255,255,140});

            Entity *e = &ship;
            DrawTextureV(ship.sprite,
                    { .x = e->pos.x - ship.sprite.width*0.5, .y = e->pos.y - ship.sprite.height*0.5 },
                    WHITE);
            Rectangle hitbox =
            {
                .x = e->pos.x - e->half_width, .y = e->pos.y - e->half_height,
                .width = TIMES2(e->half_width),
                .height = TIMES2(e->half_height),
            };
            DrawRectangleLinesEx(hitbox, 1.0, YELLOW);

            for(int i = 0; i < enemy_buf_pos; ++i) {
                if(live_enemies[i] == false) continue;

                Enemy *e = enemy_buf + i;

                DrawTextureV(e->sprite,
                        {
                        .x = e->pos.x - e->sprite.width*0.5,
                        .y = e->pos.y - e->sprite.height*0.5
                        },
                        WHITE);

                Rectangle hitbox =
                {
                    .x = e->pos.x - e->half_width, .y = e->pos.y - e->half_height,
                    .width = TIMES2(e->half_width),
                    .height = TIMES2(e->half_height),
                };
                DrawRectangleLinesEx(hitbox, 1.0, YELLOW);
            }

            for(int i = 0; i < ship.missile_launcher.pos; ++i) {
                if(ship.missile_launcher.live[i] == false) continue;

                Missile *m = ship.missile_launcher.buf + i;
                Rectangle rec =
                {
                    .x = m->pos.x - m->half_width, .y = m->pos.y - m->half_height,
                    .width = m->rec.width, .height = m->rec.height,
                };
                DrawRectangleRec(rec, m->color);
            }

            for(int enemy_index = 0; enemy_index < enemy_buf_pos; ++enemy_index) {
                if(live_enemies[enemy_index] == false) continue;

                Enemy *e = enemy_buf + enemy_index;

                for(int i = 0; i < e->missile_launcher.pos; ++i) {
                    if(e->missile_launcher.live[i] == false) continue;

                    Missile *m = e->missile_launcher.buf + i;
                    Rectangle rec =
                    {
                        .x = m->pos.x - m->half_width, .y = m->pos.y - m->half_height,
                        .width = m->rec.width, .height = m->rec.height,
                    };
                    DrawRectangleRec(rec, m->color);
                }
            }

        }

        EndMode2D();

        stbsp_sprintf(buf, "FPS: %i, FRAMETIME: %f\n", GetFPS(), timestep);
        DrawText(buf, 20, 20, 20, RAYWHITE);

        EndDrawing();
    }

    UnloadTexture(ship.sprite);
    UnloadTexture(enemy_sprite);

    CloseWindow();

    return 0;
}
