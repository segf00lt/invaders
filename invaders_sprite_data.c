
/////////////////////////
// BEGIN GENERATED


/* sprite frames array */

const Sprite_frame __sprite_frames[44] =
{
  [0] = { .x = 64, .y = 96, .w = 16, .h = 16, },
  [1] = { .x = 0, .y = 0, .w = 32, .h = 32, },
  [2] = { .x = 0, .y = 0, .w = 32, .h = 32, },
  [3] = { .x = 64, .y = 64, .w = 32, .h = 32, },
  [4] = { .x = 96, .y = 64, .w = 32, .h = 32, },
  [5] = { .x = 128, .y = 64, .w = 32, .h = 32, },
  [6] = { .x = 128, .y = 64, .w = 32, .h = 32, },
  [7] = { .x = 160, .y = 64, .w = 32, .h = 32, },
  [8] = { .x = 0, .y = 96, .w = 32, .h = 32, },
  [9] = { .x = 32, .y = 96, .w = 32, .h = 32, },
  [10] = { .x = 0, .y = 64, .w = 32, .h = 32, },
  [11] = { .x = 160, .y = 32, .w = 32, .h = 32, },
  [12] = { .x = 32, .y = 64, .w = 32, .h = 32, },
  [13] = { .x = 128, .y = 32, .w = 32, .h = 32, },
  [14] = { .x = 96, .y = 32, .w = 32, .h = 32, },
  [15] = { .x = 64, .y = 32, .w = 32, .h = 32, },
  [16] = { .x = 64, .y = 128, .w = 16, .h = 16, },
  [17] = { .x = 48, .y = 128, .w = 16, .h = 16, },
  [18] = { .x = 32, .y = 128, .w = 16, .h = 16, },
  [19] = { .x = 16, .y = 128, .w = 16, .h = 16, },
  [20] = { .x = 0, .y = 128, .w = 16, .h = 16, },
  [21] = { .x = 176, .y = 112, .w = 16, .h = 16, },
  [22] = { .x = 160, .y = 112, .w = 16, .h = 16, },
  [23] = { .x = 144, .y = 112, .w = 16, .h = 16, },
  [24] = { .x = 128, .y = 112, .w = 16, .h = 16, },
  [25] = { .x = 112, .y = 112, .w = 16, .h = 16, },
  [26] = { .x = 96, .y = 112, .w = 16, .h = 16, },
  [27] = { .x = 80, .y = 112, .w = 16, .h = 16, },
  [28] = { .x = 64, .y = 112, .w = 16, .h = 16, },
  [29] = { .x = 176, .y = 96, .w = 16, .h = 16, },
  [30] = { .x = 160, .y = 96, .w = 16, .h = 16, },
  [31] = { .x = 144, .y = 96, .w = 16, .h = 16, },
  [32] = { .x = 128, .y = 96, .w = 16, .h = 16, },
  [33] = { .x = 112, .y = 96, .w = 16, .h = 16, },
  [34] = { .x = 96, .y = 96, .w = 16, .h = 16, },
  [35] = { .x = 80, .y = 96, .w = 16, .h = 16, },
  [36] = { .x = 32, .y = 32, .w = 32, .h = 32, },
  [37] = { .x = 0, .y = 32, .w = 32, .h = 32, },
  [38] = { .x = 160, .y = 0, .w = 32, .h = 32, },
  [39] = { .x = 128, .y = 0, .w = 32, .h = 32, },
  [40] = { .x = 0, .y = 32, .w = 32, .h = 32, },
  [41] = { .x = 96, .y = 0, .w = 32, .h = 32, },
  [42] = { .x = 64, .y = 0, .w = 32, .h = 32, },
  [43] = { .x = 32, .y = 0, .w = 32, .h = 32, },
};


/* keyframes */

const s32 SPRITE_KEYFRAME_INVADER_SPIT_START = 2;
const s32 SPRITE_KEYFRAME_INVADER_SPIT_FIRED = 5;


/* sprites */

const Sprite SPRITE_INVADER_IDLE = { .flags = SPRITE_FLAG_STILL, .first_frame = 1, .last_frame = 1, .total_frames = 1 };
const Sprite SPRITE_INVADER_SPITTING = { .flags = 0, .first_frame = 2, .last_frame = 7, .fps = 6, .total_frames = 6 };
const Sprite SPRITE_ORANGE_EXPLOSION_MAIN = { .flags = 0, .first_frame = 11, .last_frame = 15, .fps = 10, .total_frames = 5 };
const Sprite SPRITE_ORANGE_FIRE_PARTICLE_MAIN = { .flags = 0, .first_frame = 16, .last_frame = 25, .fps = 33, .total_frames = 10 };
const Sprite SPRITE_PURPLE_FIRE_PARTICLE_MAIN = { .flags = 0, .first_frame = 26, .last_frame = 35, .fps = 33, .total_frames = 10 };
const Sprite SPRITE_SHIP_STRAFE = { .flags = 0, .first_frame = 37, .last_frame = 39, .fps = 7, .total_frames = 3 };
const Sprite SPRITE_SHIP_IDLE = { .flags = SPRITE_FLAG_STILL, .first_frame = 37, .last_frame = 37, .total_frames = 1 };
const Sprite SPRITE_HEALTH = { .flags = SPRITE_FLAG_STILL, .first_frame = 0, .last_frame = 0, .total_frames = 1 };
const Sprite SPRITE_INVADER_PLASMA_MISSILE = { .flags = SPRITE_FLAG_INFINITE_REPEAT, .first_frame = 8, .last_frame = 10, .fps = 10, .total_frames = 3 };
const Sprite SPRITE_SHIELDS_PICKUP = { .flags = SPRITE_FLAG_STILL, .first_frame = 36, .last_frame = 36, .total_frames = 1 };
const Sprite SPRITE_SHIP_HEALTH_ICON = { .flags = SPRITE_FLAG_STILL, .first_frame = 40, .last_frame = 40, .total_frames = 1 };
const Sprite SPRITE_SHIP_MISSILE = { .flags = SPRITE_FLAG_INFINITE_REPEAT, .first_frame = 41, .last_frame = 43, .fps = 10, .total_frames = 3 };


/////////////////////////
// END GENERATED

