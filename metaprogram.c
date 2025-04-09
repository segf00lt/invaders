#include <raylib.h>
#include "basic.h"
#include "arena.h"
#include "str.h"
#include "json.h"
#include "aseprite.h"
#include "sprite.h"
#include "array.h"


#define ATLAS_IMAGE_PATH "./aseprite/atlas.png"
#define ATLAS_METADATA_PATH "./aseprite/atlas.json"

typedef struct File_frame_range {
  Str8 file_title;
  s64 first_frame;
  s64 last_frame;
} File_frame_range;


DECL_ARR_TYPE(File_frame_range);
DECL_SLICE_TYPE(Aseprite_atlas_frame);


void print_json_(Arena *a, JSON_value *val, int indent);
void print_json(JSON_value *val);

Color color_from_hexcode(Str8 hexcode);


INLINE void print_json(JSON_value *val) {
  Arena tmp;
  arena_init(&tmp);
  print_json_(&tmp, val, 0);
  arena_destroy(&tmp);
}

void print_json_(Arena *a, JSON_value *val, int indent) {
  char *indent_str = " ";
  int indent_factor = 4;

  Arena_save save = arena_to_save(a);
  Str8 s = {0};

  do {
    s = push_str8f(a, "%S%*s%p\n", s, indent * indent_factor, indent_str, (void*)val);
    s = push_str8f(a, "%S%*skind: %s\n", s, indent * indent_factor, indent_str, JSON_value_kind_strings[val->kind]);
    s = push_str8f(a, "%S%*sname: %.*s\n", s, indent * indent_factor, indent_str, (int)val->name.len, val->name.s);
    s = push_str8f(a, "%S%*svalue: %p\n", s, indent * indent_factor, indent_str, val->value);
    s = push_str8f(a, "%S%*sstr: %.*s\n", s, indent * indent_factor, indent_str, (int)val->str.len, val->str.s);
    s = push_str8f(a, "%S%*sinteger: %li\n", s, indent * indent_factor, indent_str, val->integer);
    s = push_str8f(a, "%S%*sfloating: %f\n", s, indent * indent_factor, indent_str, val->floating);
    s = push_str8f(a, "%S%*sparent: %p\n", s, indent * indent_factor, indent_str, val->parent);
    s = push_str8f(a, "%S%*snext: %p\n", s, indent * indent_factor, indent_str, val->next);
    s = push_str8f(a, "%S%*sprev: %p\n", s, indent * indent_factor, indent_str, val->prev);
    s = push_str8f(a, "%S\n", s);

    if(val->kind == JSON_VALUE_KIND_OBJECT || val->kind == JSON_VALUE_KIND_ARRAY) {
      print_json_(a, val->value, indent + 1);
    }
    val = val->next;
  } while(val);

  printf("%s", s.s);

  arena_from_save(a, save);
}

Color color_from_hexcode(Str8 hexcode) {
  ASSERT(hexcode.s[0] == '#');
  ASSERT(hexcode.len & 0x1);

  u8 components[4] = { 0, 0, 0, 0xff, };

  for(int i = 1, c = 0; i < hexcode.len; i += 2, c++) {
    u8 a = hexcode.s[i];
    u8 b = hexcode.s[i+1];

    components[c] = (hexdigit_to_int(a) << 4) + hexdigit_to_int(b);
  }

  Color result = { .r = components[0], .g = components[1], .b = components[2], .a = components[3] };

  return result;
}

int main(void) {
  Arena temp_arena;
  arena_init(&temp_arena);

  Arena json_arena;
  arena_init(&json_arena);

  JSON_parser json_parser;

  TraceLog(LOG_INFO, "generating sprite atlas png and metadata");
  
  ASSERT(!system("aseprite -b ./aseprite/*.aseprite --sheet-pack --list-tags --filename-format '{title}/{frame}' --tagname-format '{title}/{tag}' --sheet "ATLAS_IMAGE_PATH" --format json-array --data "ATLAS_METADATA_PATH));

  s64 src_len = 0;
  u8 *src = LoadFileData(ATLAS_METADATA_PATH, (int*)&src_len);

  json_init_parser(&json_parser, &json_arena, src, src_len);
  JSON_value *val = json_parse(&json_parser);

  if(!val) {
    if(json_parser.err) {
      PANIC("error in parsing json");
    }
  }

  //print_json(json_parser.root);

  Arena atlas_arena;
  arena_init(&atlas_arena);

  Aseprite_atlas *atlas = arena_alloc(&atlas_arena, sizeof(Aseprite_atlas));
  JSON_value *frames = val->value;

  ASSERT(str8_match(str8_lit("frames"), frames->name));
  ASSERT(frames->kind == JSON_VALUE_KIND_ARRAY);
  ASSERT(frames->next);

  TraceLog(LOG_INFO, "atlas has %li frames", frames->array_length);

  atlas->frames_count = frames->array_length;
  atlas->frames = arena_alloc(&atlas_arena, sizeof(Aseprite_atlas_frame) * atlas->frames_count);

  int frame_i = 0;
  JSON_value *frame = frames->value;
  for(;frame; frame = frame->next, frame_i++) {
    ASSERT(frame->kind == JSON_VALUE_KIND_OBJECT);
    ASSERT(frame->name.s == NULL);
    ASSERT(frame->object_child_count == 7);
    ASSERT(frame->value);

    JSON_value *field = frame->value;

    Aseprite_atlas_frame atlas_frame = {0};

    for(; field; field = field->next) {

      if(str8_match_lit("filename", field->name)) {
        ASSERT(field->kind == JSON_VALUE_KIND_STRING);

        Arena_save save = arena_to_save(&atlas_arena);

        Str8_list list = str8_split_by_char(&atlas_arena, field->str, '/');

        ASSERT(list.count == 2);

        if(!str8_is_cident(list.first->str)) {
          TraceLog(LOG_ERROR, "file '%.*s.aseprite' has an invalid name, file names must start with a letter or underscore and be followed by any number of letters, underscores or digits", (int)list.first->str.len, list.first->str.s);
          return 1;
        }

        Str8 frame_index_str = list.last->str;

        ASSERT(str8_is_decimal(frame_index_str));

        for(int i = 0; i < frame_index_str.len; i++) {
          atlas_frame.frame_index *= 10;
          atlas_frame.frame_index += frame_index_str.s[i] - '0';
        }

        arena_from_save(&atlas_arena, save);

        atlas_frame.file_title = push_str8_copy(&atlas_arena, list.first->str);

      } else if(str8_match_lit("frame", field->name)) {
        ASSERT(field->kind == JSON_VALUE_KIND_OBJECT);
        atlas_frame.frame = aseprite_rectangle_from_json_object(field);

      } else if(str8_match_lit("rotated", field->name)) {
        ASSERT(field->kind == JSON_VALUE_KIND_BOOL);
        atlas_frame.rotated = field->boolean;

      } else if(str8_match_lit("trimmed", field->name)) {
        ASSERT(field->kind == JSON_VALUE_KIND_BOOL);
        atlas_frame.trimmed = field->boolean;

      } else if(str8_match_lit("spriteSourceSize", field->name)) {
        ASSERT(field->kind == JSON_VALUE_KIND_OBJECT);
        atlas_frame.sprite_source_size = aseprite_rectangle_from_json_object(field);

      } else if(str8_match_lit("sourceSize", field->name)) {
        ASSERT(field->kind == JSON_VALUE_KIND_OBJECT);
        atlas_frame.source_size = aseprite_vector2_wh_from_json_object(field);

      } else if(str8_match_lit("duration", field->name)) {
        ASSERT(field->kind == JSON_VALUE_KIND_NUMBER);
        atlas_frame.duration = field->integer;
      }

    }

    atlas->frames[frame_i] = atlas_frame;

  } /* for(;frame; frame = frame->next, frame_i++) */

  JSON_value *meta = frames->next;
  ASSERT(str8_match(str8_lit("meta"), meta->name));
  ASSERT(meta->value);

  { /* populate atlas meta */
    Aseprite_atlas_meta atlas_meta = {0};
    for(JSON_value *field = meta->value; field; field = field->next) {

      if(str8_match_lit("app", field->name)) {
        ASSERT(field->kind == JSON_VALUE_KIND_STRING);
        atlas_meta.app = push_str8_copy(&atlas_arena, field->str);

      } else if(str8_match_lit("version", field->name)) {
        ASSERT(field->kind == JSON_VALUE_KIND_STRING);
        atlas_meta.version = push_str8_copy(&atlas_arena, field->str);

      } else if(str8_match_lit("image", field->name)) {
        ASSERT(field->kind == JSON_VALUE_KIND_STRING);
        atlas_meta.image = push_str8_copy(&atlas_arena, field->str);

      } else if(str8_match_lit("format", field->name)) {
        ASSERT(field->kind == JSON_VALUE_KIND_STRING);
        atlas_meta.format = push_str8_copy(&atlas_arena, field->str);

      } else if(str8_match_lit("scale", field->name)) {
        ASSERT(field->kind == JSON_VALUE_KIND_STRING);
        atlas_meta.scale = push_str8_copy(&atlas_arena, field->str);

      } else if(str8_match_lit("size", field->name)) {
        ASSERT(field->kind == JSON_VALUE_KIND_OBJECT);
        atlas_meta.size = aseprite_vector2_wh_from_json_object(field);

      } else if(str8_match_lit("frameTags", field->name)) {
        ASSERT(field->kind == JSON_VALUE_KIND_ARRAY);
        atlas_meta.frame_tags_count = field->array_length;
        atlas_meta.frame_tags =
          arena_alloc(&atlas_arena, sizeof(Aseprite_frame_tag) * atlas_meta.frame_tags_count);

        int tag_i = 0;
        JSON_value *tag = field->value;
        for(;tag; tag = tag->next, tag_i++) {
          ASSERT(tag->kind == JSON_VALUE_KIND_OBJECT);
          ASSERT(tag->name.s == 0);

          Aseprite_frame_tag atlas_tag = {0};

          JSON_value *tag_field = tag->value;
          for(;tag_field; tag_field = tag_field->next) {

            if(str8_match_lit("name", tag_field->name)) {
              ASSERT(tag_field->kind == JSON_VALUE_KIND_STRING);

              Arena_save save = arena_to_save(&atlas_arena);

              Str8_list list = str8_split_by_char(&atlas_arena, tag_field->str, '/');

              ASSERT(list.count == 2);

              if(!str8_is_cident(list.first->str)) {
                TraceLog(LOG_ERROR, "file '%.*s.aseprite' has an invalid name, filenames must start with a letter or underscore and be followed by any number of letters, underscores or digits", (int)list.first->str.len, list.first->str.s);
                return 1;
              }

              if(!str8_is_cident(list.last->str)) {
                TraceLog(LOG_ERROR, "the tag '%.*s' in file '%.*s.aseprite' has an invalid name, tag names must start with a letter or underscore and be followed by any number of letters, underscores or digits", (int)list.last->str.len, list.last->str.s, (int)list.first->str.len, list.first->str.s);
                return 1;
              }

              arena_from_save(&atlas_arena, save);

              atlas_tag.file_title = push_str8_copy(&atlas_arena, list.first->str);
              atlas_tag.tag_name = push_str8_copy(&atlas_arena, list.last->str);

            } else if(str8_match_lit("data", tag_field->name)) {
              ASSERT(tag_field->kind == JSON_VALUE_KIND_STRING);

              if(str8_match_lit("keyframe", tag_field->str)) {
                atlas_tag.is_keyframe = 1;
              } else if(tag_field->str.len > 0) {
                ASSERT(atlas_tag.tag_name.s && atlas_tag.file_title.s);

                TraceLog(LOG_WARNING, "tag '%s' in file '%s.aseprite' has an unrecognized string '%s' in the data field", atlas_tag.tag_name.s, atlas_tag.file_title.s, tag_field->str.s);
              }

            } else if(str8_match_lit("repeat", tag_field->name)) {
              ASSERT(tag_field->kind == JSON_VALUE_KIND_STRING);

              Str8 repeat_str = tag_field->str;

              ASSERT(str8_is_decimal(repeat_str));

              for(int i = 0; i < repeat_str.len; i++) {
                atlas_tag.n_repeats *= 10;
                atlas_tag.n_repeats += repeat_str.s[i] - '0';
              }

            } else if(str8_match_lit("from", tag_field->name)) {
              ASSERT(tag_field->kind == JSON_VALUE_KIND_NUMBER);
              atlas_tag.from = tag_field->integer;

            } else if(str8_match_lit("to", tag_field->name)) {
              ASSERT(tag_field->kind == JSON_VALUE_KIND_NUMBER);
              atlas_tag.to = tag_field->integer;

            } else if(str8_match_lit("direction", tag_field->name)) {
              ASSERT(tag_field->kind == JSON_VALUE_KIND_STRING);
              for(int i = 0; i < ARRLEN(Aseprite_anim_dir_lower_strings); i++) {
                if(str8_match(Aseprite_anim_dir_lower_strings[i], tag_field->str)) {
                  atlas_tag.direction = (Aseprite_anim_dir)i;
                }
              }

            } else if(str8_match_lit("color", tag_field->name)) {
              ASSERT(tag_field->kind == JSON_VALUE_KIND_STRING);
              Str8 hexcode = tag_field->str;
              atlas_tag.color = color_from_hexcode(hexcode);
            }

          }

          atlas_meta.frame_tags[tag_i] = atlas_tag;

        }

      }

    }

    atlas->meta = atlas_meta;

  } /* populate atlas meta */

  arena_destroy(&json_arena);


  Arena sprite_arena;
  arena_init(&sprite_arena);

  { /* generate sprites from aseprite atlas */

    /* NOTE
     *
     * All the data generated by aseprite is used to generate a .c file
     * containing the frames for all the sprites, all the keyframes, and all the Sprite structs (animated or not).
     * This file is included in the game code.
     *
     * [X] All frames in a .aseprite file must be either tagged or not, otherwise we report an error and the game
     *     won't build. But, if the tags happen to be keyframes, then it's allowed, but only under this condition.
     *
     * [X] Tags with the data field set to "keyframe" will be used to generate keyframe variables of the same name as the tag,
     *     this allows quick checking of the current state of an entities animation within gameplay code.
     *
     * [X] All file titles and tag names must be valid C variable names.
     *
     * [X] If a tag has no user data and only contains a single frame it will be assumed as a static sprite, no animation will apply.
     *
     * [X] In any other case, the tag will correspond to an animation with a certain name.
     *
     * [X] If a file has no tags in it, a Sprite struct will be generated using the title of that file, previous rules also applying.
     *
     * [X] All frames in an animation should have the same duration.
     *
     * [X] If the repeats field was not set or is zero, then the animation will repeat infinitely.
     *     Apart from this, the n_repeats field of Aseprite_frame_tag is not used.
     *
     */

    { /* check keyframes only span 1 frame */

      for(int i = 0; i < atlas->meta.frame_tags_count; i++) {
        Aseprite_frame_tag tag = atlas->meta.frame_tags[i];

        if(tag.is_keyframe && tag.to - tag.from > 0) {
          TraceLog(LOG_ERROR, "in file '%s.aseprite' keyframe '%s' has range { .from = %li, .to = %li }, a keyframe must only apply to a single frame",
              tag.file_title.s, tag.tag_name.s, tag.from, tag.to);
          return 1;
        }
      }

    } /* check keyframes only span 1 frame */

    Sprite_frame_slice sprite_frames =
    {
      .d = arena_alloc(&sprite_arena, sizeof(Sprite_frame) * atlas->frames_count),
      .count = atlas->frames_count,
    };

    for(int i = 0; i < atlas->frames_count; i++) {
      Rectangle frame = atlas->frames[i].frame;

      sprite_frames.d[i] = 
        (Sprite_frame) {
          .x = (u16)frame.x,
          .y = (u16)frame.y,
          .w = (u16)frame.width,
          .h = (u16)frame.height,
        };

    }

    Arena code_arena;
    arena_init(&code_arena);

    Str8 generated_code = 
      push_str8f(&code_arena,
          "\n/////////////////////////\n"
          "// BEGIN GENERATED\n\n");

    generated_code =
      push_str8f(&code_arena, "%S\n/* sprite frames array */\n\nconst Sprite_frame __sprite_frames[%li] =\n{\n", generated_code, sprite_frames.count);
    for(int i = 0; i < sprite_frames.count; i++) {
      Sprite_frame f = sprite_frames.d[i];
      generated_code =
        push_str8f(&code_arena, "%S  { .x = %u, .y = %u, .w = %u, .h = %u, },\n", generated_code, f.x, f.y, f.w, f.h);
    }
    generated_code =
      push_str8f(&code_arena, "%S};\n\n", generated_code);


    Str8_list all_sprite_files = {0};
    {
      Slice(Aseprite_atlas_frame) frames = { .d = atlas->frames, .count = atlas->frames_count };
      for(int i = 0; i < frames.count; i++) {
        Aseprite_atlas_frame frame = frames.d[i];
        Str8 file_title = frame.file_title;
        str8_list_append_string(&sprite_arena, all_sprite_files, file_title);

        while(i+1 < frames.count) {
          Aseprite_atlas_frame frame = frames.d[i+1];
          if(!str8_match(file_title, frame.file_title)) {
            break;
          }
          i++;

        }

      }

      TraceLog(LOG_INFO, "%li sprite files total", all_sprite_files.count);
      for(Str8_node *node = all_sprite_files.first; node; node = node->next) {
        TraceLog(LOG_INFO, "%s.aseprite", node->str.s);
      }
    }

    Str8_list sprite_files_with_frame_tags = {0};
    {
      Aseprite_frame_tag *frame_tags = atlas->meta.frame_tags;
      s64 frame_tags_count = atlas->meta.frame_tags_count;

      for(int i = 0; i < frame_tags_count; i++) {
        Aseprite_frame_tag tag = frame_tags[i];

        if(tag.is_keyframe) {
          continue;
        }

        Str8 file_title = tag.file_title;
        str8_list_append_string(&sprite_arena, sprite_files_with_frame_tags, file_title);

        while(i+1 < frame_tags_count) {
          Aseprite_frame_tag tag = frame_tags[i+1];
          if(!str8_match(file_title, tag.file_title)) {
            break;
          }
          i++;

        }

      }

      TraceLog(LOG_INFO, "%li sprite files with multiple animations", sprite_files_with_frame_tags.count);
      for(Str8_node *node = sprite_files_with_frame_tags.first; node; node = node->next) {
        TraceLog(LOG_INFO, "%.*s.aseprite", (int)node->str.len, node->str.s);
      }

    }

    { /* check all files with tags (non keyframe ones) are fully tagged, I.E. don't have ranges of untagged frames */

      int frame_i = 0;
      for(Str8_node *node = sprite_files_with_frame_tags.first; node; node = node->next) {

        Aseprite_atlas_frame frame;
        for(; frame_i < atlas->frames_count; frame_i++) {
          frame = atlas->frames[frame_i];
          if(str8_match(frame.file_title, node->str)) {
            break;
          }
        }

        for(; frame_i < atlas->frames_count; frame_i++) {
          frame = atlas->frames[frame_i];
          if(!str8_match(frame.file_title, node->str)) {
            frame = atlas->frames[frame_i - 1];
            break;
          }
        }

        ASSERT(frame.frame_index + 1 > 0);
        b8 visited_frames[frame.frame_index + 1];
        memory_set(visited_frames, 0, sizeof(visited_frames));

        {
          int i = 0;
          for(; i < atlas->meta.frame_tags_count; i++) {
            Aseprite_frame_tag tag = atlas->meta.frame_tags[i];
            if(str8_match(node->str, tag.file_title)) {
              break;
            }
          }

          Aseprite_frame_tag tag;
          for(; i < atlas->meta.frame_tags_count; i++) {
            tag = atlas->meta.frame_tags[i];

            if(!str8_match(node->str, tag.file_title)) {
              break;
            }

            if(tag.is_keyframe) {
              continue;
            }

            for(s64 frame_index = tag.to; frame_index <= tag.from; frame_index++) {
              visited_frames[frame_index] = 1;
            }

          }

          b8 there_are_untagged_frames = 0;
          Str8 untagged_frames_list_str = {0};
          for(int i = 0; i < ARRLEN(visited_frames); i++) {
            if(!visited_frames[i]) {
              there_are_untagged_frames = 1;
              untagged_frames_list_str = push_str8f(&temp_arena, "%S  %i", untagged_frames_list_str, i);
            }
          }

          if(there_are_untagged_frames) {
            TraceLog(LOG_WARNING, "in file '%s.aseprite', the frames %S are untagged, it is recommended to tag all frames or none, as untagged frames are ignored",
                node->str.s, untagged_frames_list_str);
          }

        }

      } /* for(Str8_node *node = sprite_files_with_frame_tags.first; node; node = node->next) */

    } /* check all files with tags (non keyframe ones) are fully tagged, I.E. don't have ranges of untagged frames */


    Arr(File_frame_range) file_frame_ranges;
    arr_init_ex(file_frame_ranges, code_arena, all_sprite_files.count);

    for(int i = 0; i < atlas->frames_count; i++) {
      Aseprite_atlas_frame frame = atlas->frames[i];
      File_frame_range range = { .file_title = frame.file_title, .first_frame = i };
      for(; i < atlas->frames_count && str8_match(frame.file_title, atlas->frames[i].file_title); i++) {}
      i--;
      range.last_frame = i;
      arr_push(file_frame_ranges, range);
    }
    TraceLog(LOG_DEBUG, "file_frame_ranges.count = %li", file_frame_ranges.count);


    { /* generate keyframes */

      generated_code =
        push_str8f(&code_arena,
            "%S\n/* keyframes */\n\n", generated_code);

      Aseprite_frame_tag *frame_tags = atlas->meta.frame_tags;
      s64 frame_tags_count = atlas->meta.frame_tags_count;

      Arena_save save = arena_to_save(&code_arena);
      Str8 keyframes_code = {0};

      for(int i = 0; i < frame_tags_count; i++) {
        Aseprite_frame_tag tag = frame_tags[i];

        if(!tag.is_keyframe) {
          continue;
        }

        ASSERT(tag.to == tag.from);

        keyframes_code =
          push_str8f(&code_arena,
              "%Sconst s32 SPRITE_KEYFRAME_%S_%S = %li;\n",
              keyframes_code, str8_to_upper(&code_arena, tag.file_title), str8_to_upper(&code_arena, tag.tag_name), tag.from);

      }

      arena_from_save(&code_arena, save);
      keyframes_code = push_str8_copy(&code_arena, keyframes_code);
      generated_code = push_str8f(&code_arena, "%S%S", generated_code, keyframes_code);

    } /* generate keyframes */

    { /* generate sprites */

      generated_code =
        push_str8f(&code_arena,
            "%S\n\n/* sprites */\n\n", generated_code);

      Arena_save save = arena_to_save(&code_arena);
      Str8 sprites_code = {0};

      for(int i = 0; i < atlas->meta.frame_tags_count; i++) {
        Aseprite_frame_tag tag = atlas->meta.frame_tags[i];

        if(tag.is_keyframe) {
          continue;
        }

        File_frame_range range = {0};
        for(int i = 0; i < file_frame_ranges.count; i++) {
          if(str8_match(file_frame_ranges.d[i].file_title, tag.file_title)) {
            range = file_frame_ranges.d[i];
            break;
          }
        }

        s64 tag_first_frame = range.first_frame + tag.from;
        s64 tag_last_frame = range.first_frame + tag.to;

        if(tag.to == tag.from) {
          sprites_code =
            push_str8f(&code_arena,
                "%Sconst Sprite SPRITE_%S_%S = { .flags = SPRITE_FLAG_STILL, .frame = %li, .total_frames = 1 };\n",
                sprites_code, str8_to_upper(&code_arena, tag.file_title), str8_to_upper(&code_arena, tag.tag_name), tag_first_frame);
        } else {
          s64 fps = 1000/atlas->frames[range.first_frame].duration;

          Str8 flags_str = {0};

          switch(tag.direction) {
            case ASEPRITE_ANIM_DIR_FORWARD:
              {
                flags_str = str8_lit("SPRITE_FLAG_DIR_FORWARD");
              } break;
            case ASEPRITE_ANIM_DIR_REVERSE:
              {
                flags_str = str8_lit("SPRITE_FLAG_DIR_REVERSE");
              } break;
            case ASEPRITE_ANIM_DIR_PINGPONG:
              {
                flags_str = str8_lit("SPRITE_FLAG_PINGPONG | SPRITE_FLAG_DIR_FORWARD");
              } break;
            case ASEPRITE_ANIM_DIR_PINGPONG_REVERSE:
              {
                flags_str = str8_lit("SPRITE_FLAG_PINGPONG | SPRITE_FLAG_DIR_REVERSE");
              } break;
          }

          if(tag.n_repeats == 0) {
            flags_str = push_str8f(&code_arena, "%S | SPRITE_FLAG_INFINITE_REPEAT", flags_str);
          }

          sprites_code =
            push_str8f(&code_arena,
                "%Sconst Sprite SPRITE_%S_%S = { .flags = %S, .first_frame = %li, .last_frame = %li, .fps = %li, .total_frames = %li };\n",
                sprites_code, str8_to_upper(&code_arena, tag.file_title), str8_to_upper(&code_arena, tag.tag_name), flags_str, tag_first_frame, tag_last_frame, fps, tag_last_frame - tag_first_frame + 1);
        }

      }

      for(int i = 0; i < file_frame_ranges.count; i++) {
        File_frame_range range = file_frame_ranges.d[i];
        b8 skip = 0;
        for(Str8_node *node = sprite_files_with_frame_tags.first; node; node = node->next) {
          if(str8_match(node->str, range.file_title)) {
            skip = 1;
            break;
          }
        }

        if(skip) {
          continue;
        }

        if(range.first_frame == range.last_frame) {
          sprites_code =
            push_str8f(&code_arena,
                "%Sconst Sprite SPRITE_%S = { .flags = SPRITE_FLAG_STILL, .frame = %li, .total_frames = 1 };\n",
                sprites_code, str8_to_upper(&code_arena, range.file_title), range.first_frame);
        } else {

          for(s64 fi = range.first_frame; fi < range.last_frame; fi++) {
            if(atlas->frames[range.first_frame].duration != atlas->frames[fi].duration) {
              TraceLog(LOG_ERROR, "in file '%s.aseprite', frame %li does not have the same duration as the overall animation, make sure you've set a constant frame rate in aseprite",
                  range.file_title.s, fi);
              return 1;
            }
          }

          s64 fps = 1000/atlas->frames[range.first_frame].duration;

          sprites_code =
            push_str8f(&code_arena,
                "%Sconst Sprite SPRITE_%S = { .flags = SPRITE_FLAG_DIR_FORWARD | SPRITE_FLAG_INFINITE_REPEAT, .first_frame = %li, .last_frame = %li, .fps = %li, .total_frames = %li };\n",
                sprites_code, str8_to_upper(&code_arena, range.file_title), range.first_frame, range.last_frame, fps, range.last_frame - range.first_frame + 1);
        }

      }

      arena_from_save(&code_arena, save);
      sprites_code = push_str8_copy(&code_arena, sprites_code);
      generated_code = push_str8f(&code_arena, "%S%S", generated_code, sprites_code);

    } /* generate sprites */

    generated_code =
      push_str8f(&code_arena,
          "%S\n\n/////////////////////////\n"
          "// END GENERATED\n\n", generated_code);

    SaveFileData("invaders_sprite_data.c", generated_code.s, generated_code.len);

    arena_destroy(&code_arena);

  } /* generate sprites from aseprite atlas */


  arena_destroy(&atlas_arena);

  arena_destroy(&sprite_arena);

  arena_destroy(&temp_arena);

  UnloadFileData(src);

  return 0;
}
