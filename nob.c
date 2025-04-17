#define _UNITY_BUILD_

#define NOB_IMPLEMENTATION

#include "nob.h"
#include "basic.h"


#define CC "clang"
#define BASE_FLAGS "-g", "-O0", "-Wall", "-Wpedantic", "-Werror", "-Wno-switch", "-Wno-comment", "-Wno-format-pedantic", "-Wno-extra-semi", "-D_UNITY_BUILD_"
#define WASM_FLAGS "-Os", "-Wall", "-Wpedantic", "-Werror", "-Wno-switch", "-Wno-comment", "-Wno-format-pedantic", "-Wno-extra-semi", "-D_UNITY_BUILD_"
#define TARGET "invaders.c"
#define EXE "invaders"
#define LDFLAGS "-lraylib", "-lm"
#define METAPROGRAM_EXE "metaprogram"

#ifdef __MACH__

#define GAME_MODULE "invaders.dylib"
#define SHARED "-dynamiclib"
#define EXTRA_FLAGS "-L/opt/homebrew/lib/", "-I/opt/homebrew/include/", "-Wno-format"

#else

#define GAME_MODULE "invaders.so"
#define SHARED "-shared"
#define EXTRA_FLAGS "-L./third_party/raylib/", "-I./third_party/raylib/"

#endif

#define RAYLIB_DYNAMIC_LINK_OPTIONS "-L./third_party/raylib/", "-lraylib", "-Wl,-rpath,./third_party/raylib/"
#define RAYLIB_STATIC_LINK_OPTIONS "./third_party/raylib/libraylib.a"
#define RAYLIB_STATIC_LINK_WASM_OPTIONS "./third_party/raylib/libraylib.web.a"

#define RAYLIB_HEADERS "third_party/raylib/raylib.h", "third_party/raylib/raymath.h", "third_party/raylib/rlgl.h"
#define FLAGS BASE_FLAGS, EXTRA_FLAGS


int rule_metaprogram(void);
int rule_run_metaprogram(void);
int rule_hot_reload(void);
int rule_static(void);
int rule_wasm(void);
int rule_tags(void);


int rule_metaprogram(void) {
  nob_log(NOB_INFO, "building metaprogram");

  rule_tags();

  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, CC, FLAGS, "-fPIC", "metaprogram.c", "-o", METAPROGRAM_EXE, LDFLAGS);

  if(!nob_cmd_run_sync(cmd)) return 0;

  return 1;
}

int rule_run_metaprogram(void) {
  nob_log(NOB_INFO, "running metaprogram");

  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, "./metaprogram");

  if(!nob_cmd_run_sync(cmd)) return 0;

  return 1;
}

int rule_hot_reload(void) {
  rule_run_metaprogram();

  rule_tags();

  Nob_Cmd cmd = {0};

  nob_log(NOB_INFO, "building in hot reload mode");

  nob_cmd_append(&cmd, CC, BASE_FLAGS, "-fPIC", "main.c", RAYLIB_DYNAMIC_LINK_OPTIONS, "-o", EXE, "-lm");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, CC, BASE_FLAGS, "-fPIC", SHARED, "invaders.c", RAYLIB_DYNAMIC_LINK_OPTIONS, "-o", GAME_MODULE, "-lm");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}

int rule_static(void) {
  rule_run_metaprogram();

  rule_tags();

  Nob_Cmd cmd = {0};

  nob_log(NOB_INFO, "building in static mode");

  nob_cmd_append(&cmd, CC, BASE_FLAGS, "static_main.c", RAYLIB_STATIC_LINK_OPTIONS, "-o", EXE, "-lm");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}

// TODO WASM build has no audio and the nightsky.png background isn't scrolling properly
int rule_wasm(void) {
  rule_run_metaprogram();
  rule_tags();

  Nob_Cmd cmd = {0};

  nob_log(NOB_INFO, "building for WASM");

  // -sUSE_GLFW=3 -sTOTAL_MEMORY=67108864 -sFORCE_FILESYSTEM=1 --preload-file resources -sMINIFY_HTML=0
  char *target = "wasm_main.c";
  nob_cmd_append(&cmd, "emcc", WASM_FLAGS, "-I./third_party/raylib/", "--preload-file", "./aseprite/", "--preload-file", "./sprites/", "--preload-file", "./sounds/", target, RAYLIB_STATIC_LINK_WASM_OPTIONS, "-L"RAYLIB_STATIC_LINK_WASM_OPTIONS, "-sEXPORTED_RUNTIME_METHODS=ccall", "-sUSE_GLFW=3", "-sFORCE_FILESYSTEM=1", "-sMINIFY_HTML=0", "--shell-file", "./third_party/raylib/minshell.html", "-o", "invaders.html");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}

int rule_tags(void) {
  Nob_Cmd cmd = {0};

  nob_cmd_append(&cmd, "ctags", "-w", "--language-force=c", "--c-kinds=+zfx", "--extras=+q", "--fields=+n", "--exclude='*.h'", "--exclude=third_party", "-R");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, "ctags", "-w", "--language-force=c", "--c-kinds=+zpx", "--extras=+q", "--fields=+n", "-a", "--recurse", RAYLIB_HEADERS);
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}


int main(int argc, char **argv) {

  NOB_GO_REBUILD_URSELF(argc, argv);

  if(!rule_hot_reload()) return 1;
  //if(!rule_wasm()) return 1;
  //return !rule_static();

  return 0;
}
