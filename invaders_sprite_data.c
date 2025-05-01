
/////////////////////////
// BEGIN GENERATED


/* sprite frames array */

const Sprite_frame __sprite_frames[45] =
{
  [0] = { .x = 160, .y = 112, .w = 16, .h = 16, },
  [1] = { .x = 80, .y = 128, .w = 16, .h = 16, },
  [2] = { .x = 0, .y = 64, .w = 32, .h = 32, },
  [3] = { .x = 0, .y = 64, .w = 32, .h = 32, },
  [4] = { .x = 32, .y = 64, .w = 32, .h = 32, },
  [5] = { .x = 64, .y = 64, .w = 32, .h = 32, },
  [6] = { .x = 96, .y = 64, .w = 32, .h = 32, },
  [7] = { .x = 96, .y = 64, .w = 32, .h = 32, },
  [8] = { .x = 128, .y = 64, .w = 32, .h = 32, },
  [9] = { .x = 160, .y = 64, .w = 32, .h = 32, },
  [10] = { .x = 0, .y = 96, .w = 32, .h = 32, },
  [11] = { .x = 160, .y = 32, .w = 32, .h = 32, },
  [12] = { .x = 32, .y = 96, .w = 32, .h = 32, },
  [13] = { .x = 128, .y = 32, .w = 32, .h = 32, },
  [14] = { .x = 96, .y = 32, .w = 32, .h = 32, },
  [15] = { .x = 64, .y = 32, .w = 32, .h = 32, },
  [16] = { .x = 32, .y = 32, .w = 32, .h = 32, },
  [17] = { .x = 64, .y = 128, .w = 16, .h = 16, },
  [18] = { .x = 48, .y = 128, .w = 16, .h = 16, },
  [19] = { .x = 32, .y = 128, .w = 16, .h = 16, },
  [20] = { .x = 16, .y = 128, .w = 16, .h = 16, },
  [21] = { .x = 0, .y = 128, .w = 16, .h = 16, },
  [22] = { .x = 176, .y = 112, .w = 16, .h = 16, },
  [23] = { .x = 64, .y = 96, .w = 16, .h = 16, },
  [24] = { .x = 144, .y = 112, .w = 16, .h = 16, },
  [25] = { .x = 128, .y = 112, .w = 16, .h = 16, },
  [26] = { .x = 112, .y = 112, .w = 16, .h = 16, },
  [27] = { .x = 96, .y = 112, .w = 16, .h = 16, },
  [28] = { .x = 80, .y = 112, .w = 16, .h = 16, },
  [29] = { .x = 64, .y = 112, .w = 16, .h = 16, },
  [30] = { .x = 176, .y = 96, .w = 16, .h = 16, },
  [31] = { .x = 160, .y = 96, .w = 16, .h = 16, },
  [32] = { .x = 144, .y = 96, .w = 16, .h = 16, },
  [33] = { .x = 128, .y = 96, .w = 16, .h = 16, },
  [34] = { .x = 112, .y = 96, .w = 16, .h = 16, },
  [35] = { .x = 96, .y = 96, .w = 16, .h = 16, },
  [36] = { .x = 80, .y = 96, .w = 16, .h = 16, },
  [37] = { .x = 32, .y = 0, .w = 32, .h = 32, },
  [38] = { .x = 64, .y = 0, .w = 32, .h = 32, },
  [39] = { .x = 96, .y = 0, .w = 32, .h = 32, },
  [40] = { .x = 128, .y = 0, .w = 32, .h = 32, },
  [41] = { .x = 64, .y = 0, .w = 32, .h = 32, },
  [42] = { .x = 160, .y = 0, .w = 32, .h = 32, },
  [43] = { .x = 0, .y = 32, .w = 32, .h = 32, },
  [44] = { .x = 0, .y = 0, .w = 32, .h = 32, },
};


/* keyframes */

const s32 SPRITE_KEYFRAME_INVADER_SPIT_START = 3;
const s32 SPRITE_KEYFRAME_INVADER_SPIT_FIRED = 6;


/* sprites */

const Sprite SPRITE_INVADER_IDLE = { .flags = SPRITE_FLAG_STILL, .first_frame = 2, .last_frame = 2, .total_frames = 1 };
const Sprite SPRITE_INVADER_SPITTING = { .flags = 0, .first_frame = 3, .last_frame = 8, .fps = 6, .total_frames = 6 };
const Sprite SPRITE_ORANGE_EXPLOSION_MAIN = { .flags = 0, .first_frame = 12, .last_frame = 16, .fps = 8, .total_frames = 5 };
const Sprite SPRITE_ORANGE_FIRE_PARTICLE_MAIN = { .flags = 0, .first_frame = 17, .last_frame = 26, .fps = 33, .total_frames = 10 };
const Sprite SPRITE_PURPLE_FIRE_PARTICLE_MAIN = { .flags = 0, .first_frame = 27, .last_frame = 36, .fps = 33, .total_frames = 10 };
const Sprite SPRITE_SHIP_STRAFE = { .flags = 0, .first_frame = 38, .last_frame = 40, .fps = 7, .total_frames = 3 };
const Sprite SPRITE_SHIP_IDLE = { .flags = SPRITE_FLAG_STILL, .first_frame = 38, .last_frame = 38, .total_frames = 1 };
const Sprite SPRITE_DOUBLE_MISSILE_PICKUP = { .flags = SPRITE_FLAG_STILL, .first_frame = 0, .last_frame = 0, .total_frames = 1 };
const Sprite SPRITE_HEALTH = { .flags = SPRITE_FLAG_STILL, .first_frame = 1, .last_frame = 1, .total_frames = 1 };
const Sprite SPRITE_INVADER_PLASMA_MISSILE = { .flags = SPRITE_FLAG_INFINITE_REPEAT, .first_frame = 9, .last_frame = 11, .fps = 10, .total_frames = 3 };
const Sprite SPRITE_SHIELDS_PICKUP = { .flags = SPRITE_FLAG_STILL, .first_frame = 37, .last_frame = 37, .total_frames = 1 };
const Sprite SPRITE_SHIP_HEALTH_ICON = { .flags = SPRITE_FLAG_STILL, .first_frame = 41, .last_frame = 41, .total_frames = 1 };
const Sprite SPRITE_SHIP_MISSILE = { .flags = SPRITE_FLAG_INFINITE_REPEAT, .first_frame = 42, .last_frame = 44, .fps = 10, .total_frames = 3 };


/////////////////////////
// END GENERATED

