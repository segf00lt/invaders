
/////////////////////////
// BEGIN GENERATED


/* sprite frames array */

const Sprite_frame __sprite_frames[28] =
{
  [0] = { .x = 0, .y = 64, .w = 32, .h = 32, },
  [1] = { .x = 0, .y = 64, .w = 32, .h = 32, },
  [2] = { .x = 128, .y = 96, .w = 32, .h = 32, },
  [3] = { .x = 96, .y = 96, .w = 32, .h = 32, },
  [4] = { .x = 64, .y = 96, .w = 32, .h = 32, },
  [5] = { .x = 64, .y = 96, .w = 32, .h = 32, },
  [6] = { .x = 32, .y = 96, .w = 32, .h = 32, },
  [7] = { .x = 0, .y = 96, .w = 32, .h = 32, },
  [8] = { .x = 128, .y = 64, .w = 32, .h = 32, },
  [9] = { .x = 96, .y = 64, .w = 32, .h = 32, },
  [10] = { .x = 64, .y = 64, .w = 32, .h = 32, },
  [11] = { .x = 32, .y = 64, .w = 32, .h = 32, },
  [12] = { .x = 0, .y = 0, .w = 32, .h = 32, },
  [13] = { .x = 128, .y = 32, .w = 32, .h = 32, },
  [14] = { .x = 96, .y = 32, .w = 32, .h = 32, },
  [15] = { .x = 64, .y = 32, .w = 32, .h = 32, },
  [16] = { .x = 32, .y = 32, .w = 32, .h = 32, },
  [17] = { .x = 0, .y = 32, .w = 32, .h = 32, },
  [18] = { .x = 64, .y = 32, .w = 32, .h = 32, },
  [19] = { .x = 128, .y = 0, .w = 32, .h = 32, },
  [20] = { .x = 96, .y = 0, .w = 32, .h = 32, },
  [21] = { .x = 64, .y = 0, .w = 32, .h = 32, },
  [22] = { .x = 32, .y = 0, .w = 32, .h = 32, },
  [23] = { .x = 32, .y = 0, .w = 32, .h = 32, },
  [24] = { .x = 32, .y = 0, .w = 32, .h = 32, },
  [25] = { .x = 32, .y = 0, .w = 32, .h = 32, },
  [26] = { .x = 32, .y = 0, .w = 32, .h = 32, },
  [27] = { .x = 32, .y = 0, .w = 32, .h = 32, },
};


/* keyframes */

const s32 SPRITE_KEYFRAME_INVADER_SPIT_START = 1;
const s32 SPRITE_KEYFRAME_INVADER_SPIT_FIRED = 4;
const s32 SPRITE_KEYFRAME_ORANGE_EXPLOSION_START = 10;
const s32 SPRITE_KEYFRAME_TEST_TAG = 22;


/* sprites */

const Sprite SPRITE_INVADER_IDLE = { .flags = SPRITE_FLAG_STILL, .abs_cur_frame = 0, .first_frame = 0, .last_frame = 0, .total_frames = 1 };
const Sprite SPRITE_INVADER_SPITTING = { .flags = 0, .abs_cur_frame = 6, .first_frame = 1, .last_frame = 6, .fps = 10, .total_frames = 6 };
const Sprite SPRITE_ORANGE_EXPLOSION_MAIN = { .flags = 0, .abs_cur_frame = 14, .first_frame = 10, .last_frame = 14, .fps = 10, .total_frames = 5 };
const Sprite SPRITE_SHIP_STRAFE = { .flags = 0, .abs_cur_frame = 17, .first_frame = 15, .last_frame = 17, .fps = 7, .total_frames = 3 };
const Sprite SPRITE_SHIP_IDLE = { .flags = SPRITE_FLAG_STILL, .abs_cur_frame = 15, .first_frame = 15, .last_frame = 15, .total_frames = 1 };
const Sprite SPRITE_INVADER_PLASMA_MISSILE = { .flags = SPRITE_FLAG_INFINITE_REPEAT, .abs_cur_frame = 7, .first_frame = 7, .last_frame = 9, .fps = 10, .total_frames = 3 };
const Sprite SPRITE_SHIP_HEALTH_ICON = { .flags = SPRITE_FLAG_STILL, .abs_cur_frame = 18, .first_frame = 18, .last_frame = 18, .total_frames = 1 };
const Sprite SPRITE_SHIP_MISSILE = { .flags = SPRITE_FLAG_INFINITE_REPEAT, .abs_cur_frame = 19, .first_frame = 19, .last_frame = 21, .fps = 10, .total_frames = 3 };
const Sprite SPRITE_TEST = { .flags = SPRITE_FLAG_INFINITE_REPEAT, .abs_cur_frame = 22, .first_frame = 22, .last_frame = 27, .fps = 10, .total_frames = 6 };


/////////////////////////
// END GENERATED

