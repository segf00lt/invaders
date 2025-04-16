#define NOB_IMPLEMENTATION

#include "nob.h"


#define CC "clang"
#define BASE_FLAGS "-g", "-O0", "-Wall", "-Wpedantic", "-Werror", "-Wno-switch", "-Wno-comment", "-Wno-format-pedantic", "-Wno-extra-semi", "-D_UNITY_BUILD_", "-I./third_party"
#define TARGET "invaders.c"
#define EXE "invaders"
#define LDFLAGS "-lraylib", "-lm"
#define METAPROGRAM_EXE "metaprogram"

#ifdef __MACH__

#define RAYLIB_HEADER_PATTERN "/opt/homebrew/include/raylib.h", "/opt/homebrew/include/raymath.h", "/opt/homebrew/include/rlgl.h"
#define GAME_MODULE "invaders.dylib"
#define SHARED "-dynamiclib"
#define EXTRA_FLAGS "-L/opt/homebrew/lib/", "-I/opt/homebrew/include/", "-Wno-format"

#else

#define RAYLIB_HEADER_PATTERN "/usr/local/include/raylib.h", "/usr/local/include/raymath.h", "/usr/local/include/rlgl.h"
#define GAME_MODULE "invaders.so"
#define SHARED "-shared"
#define EXTRA_FLAGS "-L/usr/local/lib/", "-I/usr/local/include/"

#endif

#define FLAGS BASE_FLAGS, EXTRA_FLAGS


int rule_first(void);
int rule_metaprogram(void);
int rule_run_metaprogram(void);
int rule_game_module(void);
int rule_all(void);
int rule_tags(void);


int rule_first(void) {
  return rule_game_module();
}

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

int rule_game_module(void) {
  rule_run_metaprogram();

  rule_tags();

  Nob_Cmd cmd = {0};

  nob_log(NOB_INFO, "building in hot reload mode");

  nob_cmd_append(&cmd, CC, FLAGS, "-fPIC", "main.c", "-o", EXE, LDFLAGS);
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, CC, FLAGS, "-fPIC", SHARED, TARGET, "-o", GAME_MODULE, LDFLAGS);
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}

int rule_all(void) {
  Nob_Cmd cmd = {0};

  nob_cmd_append(&cmd, CC, FLAGS, TARGET, "-o", EXE, LDFLAGS);
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}

int rule_tags(void) {
  Nob_Cmd cmd = {0};

  nob_cmd_append(&cmd, "ctags", "-w", "--language-force=c", "--c-kinds=+zfx", "--extras=+q", "--fields=+n", "--recurse", ".", "--exclude='*.h'");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, "ctags", "-w", "--language-force=c", "--c-kinds=+zpx", "--extras=+q", "--fields=+n", "-a", "--recurse", RAYLIB_HEADER_PATTERN);
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}


int main(int argc, char **argv)
{
  NOB_GO_REBUILD_URSELF(argc, argv);

  if(!rule_first()) return 1;

  return 0;
}
