#include <raylib.h>
#include "basic.h"
#include "arena.h"
#include "json.h"


#define ATLAS_IMAGE_PATH "./aseprite/atlas.png"
#define ATLAS_METADATA_PATH "./aseprite/atlas.json"


#define ASEPRITE_ANIM_DIRS                  \
  X(FORWARD,          forward)              \
  X(REVERSE,          reverse)              \
  X(PINGPONG,         pingpong)             \
  X(PINGPONG_REVERSE, pingpong_reverse)     \


typedef struct Aseprite_atlas_frame Aseprite_atlas_frame;
typedef struct Aseprite_atlas_meta Aseprite_atlas_meta;
typedef struct Aseprite_atlas Aseprite_atlas;
typedef struct Aseprite_frame_tag Aseprite_frame_tag;

typedef enum Aseprite_anim_dir {
  ASEPRITE_ANIM_DIR_NONE = 0,
#define X(d, ...) ASEPRITE_ANIM_DIR_##d,
  ASEPRITE_ANIM_DIRS
#undef X
    ASEPRITE_ANIM_DIR_MAX,
} Aseprite_anim_dir;

Str8 Aseprite_anim_dir_lower_strings[ASEPRITE_ANIM_DIR_MAX] = {
#define X(d, s) [ASEPRITE_ANIM_DIR_##d] = str8_lit(#s),
  ASEPRITE_ANIM_DIRS
#undef X
};


struct Aseprite_atlas_frame {
  Str8      filename;
  Rectangle frame;
  Rectangle sprite_source_size;
  Vector2   source_size;
  S32       duration;
  B32       rotated;
  B32       trimmed;
};

struct Aseprite_frame_tag {
  Str8 name;
  Str8 data;
  Str8 repeat;
  S64  from;
  S64  to;

  Aseprite_anim_dir direction;

  Color color;
};

struct Aseprite_atlas_meta {
  Str8 app;
  Str8 version;
  Str8 image;
  Str8 format;
  Str8 scale;
  Vector2 size;

  Aseprite_frame_tag *frame_tags;
  S64                 frame_tags_count;
};

struct Aseprite_atlas {
  Aseprite_atlas_frame *frames;
  S64 frames_count;
  Aseprite_atlas_meta meta;
};


void print_json_(JSON_value *val, int indent);
void print_json(JSON_value *val);
Rectangle aseprite_rectangle_from_json_object(JSON_value *v);
Vector2 aseprite_vector2_from_json_object(JSON_value *v);
Vector2 aseprite_vector2_wh_from_json_object(JSON_value *v);

Color color_from_hexcode(Str8 hexcode);

B32 str8_match(Str8 a, Str8 b);
Str8 str8_copy(Arena *a, Str8 s);
#define str8_match_lit(a_lit, b) str8_match(str8_lit(a_lit), b)
#define is_upper(c) (!!('A' <= c && c <= 'Z'))
#define to_lower(c) (is_upper(c) ? (c - 'A' + 'a') : c)
#define is_alpha(c) ('a' <= to_lower(c) && to_lower(c) <= 'z')
#define letter_index(c) ((S64)(to_lower(c) - 'a'))
#define hexdigit_to_int(c) ((S64)(is_alpha(c) ? (to_lower(c) - 'a' + 0xa) : (c - '0')))


INLINE void print_json(JSON_value *val) {
  print_json_(val, 0);
}

void print_json_(JSON_value *val, int indent) {
  char *indent_str = " ";
  int indent_factor = 4;

  do {
    printf("%*s%p\n", indent * indent_factor, indent_str, (void*)val);
    printf("%*skind: %s\n", indent * indent_factor, indent_str, JSON_value_kind_strings[val->kind]);

    printf("%*sname: %.*s\n", indent * indent_factor, indent_str, (int)val->name.len, val->name.d);
    printf("%*svalue: %p\n", indent * indent_factor, indent_str, val->value);

    printf("%*sstr: %.*s\n", indent * indent_factor, indent_str, (int)val->str.len, val->str.d);
    printf("%*sinteger: %li\n", indent * indent_factor, indent_str, val->integer);
    printf("%*sfloating: %f\n", indent * indent_factor, indent_str, val->floating);

    printf("%*sparent: %p\n", indent * indent_factor, indent_str, val->parent);
    printf("%*snext: %p\n", indent * indent_factor, indent_str, val->next);
    printf("%*sprev: %p\n", indent * indent_factor, indent_str, val->prev);
    printf("\n");

    if(val->kind == JSON_VALUE_KIND_OBJECT || val->kind == JSON_VALUE_KIND_ARRAY) {
      print_json_(val->value, indent + 1);
    }
    val = val->next;
  } while(val);

}

Color color_from_hexcode(Str8 hexcode) {
  assert(hexcode.d[0] == '#');
  assert(hexcode.len & 0x1);

  U8 components[4] = { 0, 0, 0, 0xff, };

  for(int i = 1, c = 0; i < hexcode.len; i += 2, c++) {
    U8 a = hexcode.d[i];
    U8 b = hexcode.d[i+1];

    components[c] = (hexdigit_to_int(a) << 4) + hexdigit_to_int(b);
  }

  Color result = { .r = components[0], .g = components[1], .b = components[2], .a = components[3] };

  return result;
}

Rectangle aseprite_rectangle_from_json_object(JSON_value *v) {
  Rectangle result = {0};

  JSON_value *field = v->value;

  for(; field; field = field->next) {

    if(str8_match_lit("x", field->name)) {
      result.x = (F32)field->floating;
    } else if(str8_match_lit("y", field->name)) {
      result.y = (F32)field->floating;
    } else if(str8_match_lit("w", field->name)) {
      result.width = (F32)field->floating;
    } else if(str8_match_lit("h", field->name)) {
      result.height = (F32)field->floating;
    }

  }

  return result;
}

Vector2 aseprite_vector2_wh_from_json_object(JSON_value *v) {
  Vector2 result = {0};
  JSON_value *field = v->value;

  for(; field; field = field->next) {

    if(str8_match_lit("w", field->name)) {
      result.x = (F32)field->floating;
    } else if(str8_match_lit("h", field->name)) {
      result.y = (F32)field->floating;
    }

  }

  return result;

}

Vector2 aseprite_vector2_from_json_object(JSON_value *v) {
  Vector2 result = {0};

  JSON_value *field = v->value;

  for(; field; field = field->next) {

    if(str8_match_lit("x", field->name)) {
      result.x = (F32)field->floating;
    } else if(str8_match_lit("y", field->name)) {
      result.y = (F32)field->floating;
    }

  }

  return result;

}

B32 str8_match(Str8 a, Str8 b) {
  if(a.len != b.len) {
    return 0;
  } else {
    return (B32)(memcmp(a.d, b.d, a.len) == 0);
  }
}

Str8 str8_copy(Arena *a, Str8 s) {
  U8 *d = arena_alloc(a, s.len);
  memmove(d, s.d, s.len);
  return (Str8){ .d = d, .len = s.len };
}

int main(void) {
  Arena json_arena;
  arena_init(&json_arena);
  
  JSON_parser json_parser;

  assert(!system("aseprite -b ./aseprite/*.aseprite --sheet-pack --list-tags --filename-format '{title}/{frame}' --tagname-format '{title}/{tag}' --sheet "ATLAS_IMAGE_PATH" --format json-array --data "ATLAS_METADATA_PATH));
  
  S64 src_len = 0;
  U8 *src = LoadFileData(ATLAS_METADATA_PATH, (int*)&src_len);
  
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

  {
    JSON_value *frames = val->value;

    assert(str8_match(str8_lit("frames"), frames->name));
    assert(frames->kind == JSON_VALUE_KIND_ARRAY);
    assert(frames->next);

    printf("atlas has %li frames\n", frames->array_length);

    atlas->frames_count = frames->array_length;
    atlas->frames = arena_alloc(&atlas_arena, sizeof(Aseprite_atlas_frame) * atlas->frames_count);

    int frame_i = 0;
    JSON_value *frame = frames->value;
    for(;frame; frame = frame->next, frame_i++) {
      assert(frame->kind == JSON_VALUE_KIND_OBJECT);
      assert(frame->name.d == NULL);
      assert(frame->object_child_count == 7);
      assert(frame->value);

      JSON_value *field = frame->value;

      Aseprite_atlas_frame atlas_frame = {0};

      for(; field; field = field->next) {

        if(str8_match_lit("filename", field->name)) {
          assert(field->kind == JSON_VALUE_KIND_STRING);
          atlas_frame.filename = str8_copy(&atlas_arena, field->str);

        } else if(str8_match_lit("frame", field->name)) {
          assert(field->kind == JSON_VALUE_KIND_OBJECT);
          atlas_frame.frame = aseprite_rectangle_from_json_object(field);

        } else if(str8_match_lit("rotated", field->name)) {
          assert(field->kind == JSON_VALUE_KIND_BOOL);
          atlas_frame.rotated = field->boolean;

        } else if(str8_match_lit("trimmed", field->name)) {
          assert(field->kind == JSON_VALUE_KIND_BOOL);
          atlas_frame.trimmed = field->boolean;

        } else if(str8_match_lit("spriteSourceSize", field->name)) {
          assert(field->kind == JSON_VALUE_KIND_OBJECT);
          atlas_frame.sprite_source_size = aseprite_rectangle_from_json_object(field);

        } else if(str8_match_lit("sourceSize", field->name)) {
          assert(field->kind == JSON_VALUE_KIND_OBJECT);
          atlas_frame.source_size = aseprite_vector2_wh_from_json_object(field);

        } else if(str8_match_lit("duration", field->name)) {
          assert(field->kind == JSON_VALUE_KIND_NUMBER);
          atlas_frame.duration = field->integer;
        }

      }

      atlas->frames[frame_i] = atlas_frame;

    } /* for(;frame; frame = frame->next, frame_i++) */

    for(S64 i = 0; i < atlas->frames_count; i++) {
      printf("\n");
      __builtin_dump_struct(&atlas->frames[i], printf);
    }

    JSON_value *meta = frames->next;
    assert(str8_match(str8_lit("meta"), meta->name));
    assert(meta->value);

    { /* populate atlas meta */
      Aseprite_atlas_meta atlas_meta = {0};
      for(JSON_value *field = meta->value; field; field = field->next) {

        if(str8_match_lit("app", field->name)) {
          assert(field->kind == JSON_VALUE_KIND_STRING);
          atlas_meta.app = str8_copy(&atlas_arena, field->str);

        } else if(str8_match_lit("version", field->name)) {
          assert(field->kind == JSON_VALUE_KIND_STRING);
          atlas_meta.version = str8_copy(&atlas_arena, field->str);

        } else if(str8_match_lit("image", field->name)) {
          assert(field->kind == JSON_VALUE_KIND_STRING);
          atlas_meta.image = str8_copy(&atlas_arena, field->str);

        } else if(str8_match_lit("format", field->name)) {
          assert(field->kind == JSON_VALUE_KIND_STRING);
          atlas_meta.format = str8_copy(&atlas_arena, field->str);

        } else if(str8_match_lit("scale", field->name)) {
          assert(field->kind == JSON_VALUE_KIND_STRING);
          atlas_meta.scale = str8_copy(&atlas_arena, field->str);

        } else if(str8_match_lit("size", field->name)) {
          assert(field->kind == JSON_VALUE_KIND_OBJECT);
          atlas_meta.size = aseprite_vector2_wh_from_json_object(field);

        } else if(str8_match_lit("frameTags", field->name)) {
          assert(field->kind == JSON_VALUE_KIND_ARRAY);
          atlas_meta.frame_tags_count = field->array_length;
          atlas_meta.frame_tags =
            arena_alloc(&atlas_arena, sizeof(Aseprite_frame_tag) * atlas_meta.frame_tags_count);

          int tag_i = 0;
          JSON_value *tag = field->value;
          for(;tag; tag = tag->next, tag_i++) {
            assert(tag->kind == JSON_VALUE_KIND_OBJECT);
            assert(tag->name.d == 0);

            Aseprite_frame_tag atlas_tag = {0};

            JSON_value *tag_field = tag->value;
            for(;tag_field; tag_field = tag_field->next) {

              if(str8_match_lit("name", tag_field->name)) {
                assert(tag_field->kind == JSON_VALUE_KIND_STRING);
                atlas_tag.name = str8_copy(&atlas_arena, tag_field->str);

              } else if(str8_match_lit("data", tag_field->name)) {
                assert(tag_field->kind == JSON_VALUE_KIND_STRING);
                atlas_tag.data = str8_copy(&atlas_arena, tag_field->str);

              } else if(str8_match_lit("repeat", tag_field->name)) {
                assert(tag_field->kind == JSON_VALUE_KIND_STRING);
                atlas_tag.repeat = str8_copy(&atlas_arena, tag_field->str);

              } else if(str8_match_lit("from", tag_field->name)) {
                assert(tag_field->kind == JSON_VALUE_KIND_NUMBER);
                atlas_tag.from = tag_field->integer;

              } else if(str8_match_lit("to", tag_field->name)) {
                assert(tag_field->kind == JSON_VALUE_KIND_NUMBER);
                atlas_tag.to = tag_field->integer;

              } else if(str8_match_lit("direction", tag_field->name)) {
                assert(tag_field->kind == JSON_VALUE_KIND_STRING);
                for(int i = 0; i < Arrlen(Aseprite_anim_dir_lower_strings); i++) {
                  if(str8_match(Aseprite_anim_dir_lower_strings[i], tag_field->str)) {
                    atlas_tag.direction = (Aseprite_anim_dir)i;
                  }
                }

              } else if(str8_match_lit("color", tag_field->name)) {
                assert(tag_field->kind == JSON_VALUE_KIND_STRING);
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

    __builtin_dump_struct(&(atlas->meta), printf);
    for(int i = 0; i < atlas->meta.frame_tags_count; i++) {
      __builtin_dump_struct(&(atlas->meta.frame_tags[i]), printf);
    }

    printf("here\n");

    // TODO modify stb_sprintf to print Str8
    //Str8 test_str = str8_lit("test string literal");
    //printf("%.*s", (int)test_str.len, test_str.d);

  }

  arena_destroy(&json_arena);

  arena_destroy(&atlas_arena);

  return 0;
}
