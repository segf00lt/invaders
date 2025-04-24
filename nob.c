#define _UNITY_BUILD_

#define NOB_IMPLEMENTATION

#include "nob.h"
#include "basic.h"
#include "arena.h"
#include "str.h"


#define CC "clang"
#define DEV_FLAGS "-g", "-O0", "-Wall", "-Wpedantic", "-Werror", "-Wno-switch", "-Wno-comment", "-Wno-format-pedantic", "-Wno-extra-semi", "-D_UNITY_BUILD_"
#define WASM_FLAGS "-Os", "-Wall", "-Wpedantic", "-Werror", "-Wno-switch", "-Wno-comment", "-Wno-format-pedantic", "-Wno-extra-semi", "-D_UNITY_BUILD_"
#define TARGET "invaders.c"
#define EXE "invaders"
#define LDFLAGS "-lraylib", "-lm"
#define METAPROGRAM_EXE "metaprogram"

#ifdef __MACH__

#define GAME_MODULE "invaders.dylib"
#define SHARED "-dynamiclib"

#else

#define GAME_MODULE "invaders.so"
#define SHARED "-shared"

#endif

#define RAYLIB_DYNAMIC_LINK_OPTIONS "-L./third_party/raylib/",  "-I./third_party/raylib/", "-lraylib", "-Wl,-rpath,./third_party/raylib/"
#define RAYLIB_STATIC_LINK_OPTIONS "-I./third_party/raylib/", "-L./third_party/raylib/", "./third_party/raylib/libraylib.a"
#define RAYLIB_STATIC_LINK_WASM_OPTIONS "-I./third_party/raylib/", "-L./third_party/raylib/", "./third_party/raylib/libraylib.web.a"

#define RAYLIB_HEADERS "third_party/raylib/raylib.h", "third_party/raylib/raymath.h", "third_party/raylib/rlgl.h"


int build_metaprogram(void);
int run_metaprogram(void);
int build_hot_reload(void);
int build_static(void);
int build_wasm(void);
int run_tags(void);
int build_raylib(void);

Arena _arena;
Arena *arena = &_arena;

int build_raylib(void) {
  nob_log(NOB_INFO, "building raylib");

  Nob_Cmd cmd = {0};

  Arena_save save = arena_to_save(arena);

  Str8 dir_path = push_str8_copy_cstr(arena, (char*)nob_get_current_dir_temp());

  ASSERT(nob_set_current_dir("./third_party/raylib"));

  nob_cmd_append(&cmd, "make", "clean");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, "make", "RAYLIB_SRC_PATH=.", "PLATFORM=PLATFORM_DESKTOP");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, "make", "RAYLIB_SRC_PATH=.", "PLATFORM=PLATFORM_DESKTOP", "RAYLIB_LIBTYPE=SHARED");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, "make", "clean");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, "make", "RAYLIB_SRC_PATH=.", "PLATFORM=PLATFORM_WEB");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  ASSERT(nob_set_current_dir(dir_path.s));

  arena_from_save(arena, save);

  return 1;
}

int build_metaprogram(void) {
  nob_log(NOB_INFO, "building metaprogram");

  run_tags();

  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, CC, DEV_FLAGS, "-fPIC", "metaprogram.c", "-o", METAPROGRAM_EXE, LDFLAGS);

  if(!nob_cmd_run_sync(cmd)) return 0;

  return 1;
}

int run_metaprogram(void) {
  nob_log(NOB_INFO, "running metaprogram");

  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, "./metaprogram");

  if(!nob_cmd_run_sync(cmd)) return 0;

  return 1;
}

int build_hot_reload(void) {
  run_metaprogram();

  run_tags();

  Nob_Cmd cmd = {0};

  nob_log(NOB_INFO, "building in hot reload mode");

  nob_cmd_append(&cmd, CC, DEV_FLAGS, "-fPIC", "main.c", RAYLIB_DYNAMIC_LINK_OPTIONS, "-o", EXE, "-lm");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, CC, DEV_FLAGS, "-fPIC", SHARED, "invaders.c", RAYLIB_DYNAMIC_LINK_OPTIONS, "-o", GAME_MODULE, "-lm");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}

int build_static(void) {
  run_metaprogram();

  run_tags();

  Nob_Cmd cmd = {0};

  nob_log(NOB_INFO, "building in static mode");

  nob_cmd_append(&cmd, CC, DEV_FLAGS, "static_main.c", RAYLIB_STATIC_LINK_OPTIONS, "-o", EXE, "-lm");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}

// TODO WASM build has no audio
int build_wasm(void) {
  run_metaprogram();
  run_tags();

  Nob_Cmd cmd = {0};

  nob_log(NOB_INFO, "building for WASM");

  // -sUSE_GLFW=3 -sTOTAL_MEMORY=67108864 -sFORCE_FILESYSTEM=1 --preload-file resources -sMINIFY_HTML=0
  char *target = "wasm_main.c";
  nob_cmd_append(&cmd, "emcc", WASM_FLAGS, "--preload-file", "./aseprite/", "--preload-file", "./sprites/", "--preload-file", "./sounds/", target, RAYLIB_STATIC_LINK_WASM_OPTIONS, RAYLIB_STATIC_LINK_WASM_OPTIONS, "-sEXPORTED_RUNTIME_METHODS=ccall", "-sUSE_GLFW=3", "-sFORCE_FILESYSTEM=1", "-sMINIFY_HTML=0", "--shell-file", "./third_party/raylib/minshell.html", "-o", "invaders.html");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}

int run_tags(void) {
  Nob_Cmd cmd = {0};

  nob_cmd_append(&cmd, "ctags", "-w", "--language-force=c", "--c-kinds=+zfx", "--extras=+q", "--fields=+n", "--exclude='*.h'", "--exclude=third_party", "-R");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, "ctags", "-w", "--language-force=c", "--c-kinds=+zpx", "--extras=+q", "--fields=+n", "-a", "--recurse", RAYLIB_HEADERS);
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}


int main(int argc, char **argv) {

  NOB_GO_REBUILD_URSELF(argc, argv);

  arena_init(arena);

  //if(!build_raylib()) return 1;
  if(!build_hot_reload()) return 1;
  if(!build_wasm()) return 1;
  //if(!build_static()) return 1;

  return 0;
}
