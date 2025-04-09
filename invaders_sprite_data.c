
/////////////////////////
// BEGIN GENERATED


/* sprite frames array */

const Sprite_frame __sprite_frames[28] =
{
  { .x = 0, .y = 64, .w = 32, .h = 32, },
  { .x = 0, .y = 64, .w = 32, .h = 32, },
  { .x = 128, .y = 96, .w = 32, .h = 32, },
  { .x = 96, .y = 96, .w = 32, .h = 32, },
  { .x = 64, .y = 96, .w = 32, .h = 32, },
  { .x = 64, .y = 96, .w = 32, .h = 32, },
  { .x = 32, .y = 96, .w = 32, .h = 32, },
  { .x = 0, .y = 96, .w = 32, .h = 32, },
  { .x = 128, .y = 64, .w = 32, .h = 32, },
  { .x = 96, .y = 64, .w = 32, .h = 32, },
  { .x = 64, .y = 64, .w = 32, .h = 32, },
  { .x = 32, .y = 64, .w = 32, .h = 32, },
  { .x = 0, .y = 0, .w = 32, .h = 32, },
  { .x = 128, .y = 32, .w = 32, .h = 32, },
  { .x = 96, .y = 32, .w = 32, .h = 32, },
  { .x = 64, .y = 32, .w = 32, .h = 32, },
  { .x = 32, .y = 32, .w = 32, .h = 32, },
  { .x = 0, .y = 32, .w = 32, .h = 32, },
  { .x = 64, .y = 32, .w = 32, .h = 32, },
  { .x = 128, .y = 0, .w = 32, .h = 32, },
  { .x = 96, .y = 0, .w = 32, .h = 32, },
  { .x = 64, .y = 0, .w = 32, .h = 32, },
  { .x = 32, .y = 0, .w = 32, .h = 32, },
  { .x = 32, .y = 0, .w = 32, .h = 32, },
  { .x = 32, .y = 0, .w = 32, .h = 32, },
  { .x = 32, .y = 0, .w = 32, .h = 32, },
  { .x = 32, .y = 0, .w = 32, .h = 32, },
  { .x = 32, .y = 0, .w = 32, .h = 32, },
};


/* keyframes */

const s32 SPRITE_KEYFRAME_INVADER_SPIT_START = 1;
const s32 SPRITE_KEYFRAME_INVADER_SPIT_FIRED = 4;
const s32 SPRITE_KEYFRAME_ORANGE_EXPLOSION_START = 0;
const s32 SPRITE_KEYFRAME_TEST_TAG = 0;


/* sprites */

const Sprite SPRITE_INVADER_IDLE = { .flags = SPRITE_FLAG_STILL, .frame = 0, .total_frames = 1 };
const Sprite SPRITE_INVADER_SPITTING = { .flags = SPRITE_FLAG_DIR_FORWARD, .first_frame = 1, .last_frame = 6, .fps = 10, .total_frames = 6 };
const Sprite SPRITE_ORANGE_EXPLOSION_MAIN = { .flags = SPRITE_FLAG_DIR_FORWARD, .first_frame = 10, .last_frame = 14, .fps = 10, .total_frames = 5 };
const Sprite SPRITE_SHIP_STRAFE = { .flags = SPRITE_FLAG_DIR_FORWARD | SPRITE_FLAG_INFINITE_REPEAT, .first_frame = 15, .last_frame = 17, .fps = 8, .total_frames = 3 };
const Sprite SPRITE_SHIP_IDLE = { .flags = SPRITE_FLAG_STILL, .frame = 15, .total_frames = 1 };
const Sprite SPRITE_INVADER_PLASMA_MISSILE = { .flags = SPRITE_FLAG_DIR_FORWARD | SPRITE_FLAG_INFINITE_REPEAT, .first_frame = 7, .last_frame = 9, .fps = 10, .total_frames = 3 };
const Sprite SPRITE_SHIP_HEALTH_ICON = { .flags = SPRITE_FLAG_STILL, .frame = 18, .total_frames = 1 };
const Sprite SPRITE_SHIP_MISSILE = { .flags = SPRITE_FLAG_DIR_FORWARD | SPRITE_FLAG_INFINITE_REPEAT, .first_frame = 19, .last_frame = 21, .fps = 10, .total_frames = 3 };
const Sprite SPRITE_TEST = { .flags = SPRITE_FLAG_DIR_FORWARD | SPRITE_FLAG_INFINITE_REPEAT, .first_frame = 22, .last_frame = 27, .fps = 10, .total_frames = 6 };


/////////////////////////
// END GENERATED

