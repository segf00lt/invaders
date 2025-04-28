#define _UNITY_BUILD_

#define NOB_IMPLEMENTATION

#include "nob.h"
#include "basic.h"
#include "arena.h"
#include "str.h"


#define CC "clang"
#define DEV_FLAGS "-g", "-O0", "-Wall", "-Wpedantic", "-Werror", "-Wno-switch", "-Wno-comment", "-Wno-format-pedantic", "-Wno-initializer-overrides", "-Wno-extra-semi", "-D_UNITY_BUILD_"
#define WASM_FLAGS "-Os", "-Wall", "-Wpedantic", "-Werror", "-Wno-switch", "-Wno-comment", "-Wno-format-pedantic", "-Wno-initializer-overrides", "-Wno-extra-semi", "-D_UNITY_BUILD_"
#define TARGET "invaders.c"
#define EXE "invaders"
#define LDFLAGS "-lraylib", "-lm"

#if defined(OS_MAC)
#define CTAGS "/opt/homebrew/bin/ctags"
#elif defined(OS_LINUX)
#define CTAGS "/usr/bin/ctags"
#else
#error "unsupported operating system"
#endif

#ifdef __MACH__
#define STATIC_BUILD_LDFLAGS "-lm", "-framework", "IOKit", "-framework", "Cocoa", "-framework", "OpenGL"
#else
#define STATIC_BUILD_LDFLAGS "-lm"
#endif

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
int build_raylib_static(void);
int build_raylib_shared(void);
int build_raylib_web(void);
int generate_vim_project_file(void);

Arena *scratch;

#ifdef OS_WINDOWS
char project_root_path[MAX_PATH];
#else
char project_root_path[PATH_MAX];
#endif

int build_raylib(void) {
  nob_log(NOB_INFO, "building raylib");

  Nob_Cmd cmd = {0};

  ASSERT(nob_set_current_dir("./third_party/raylib"));

  nob_cmd_append(&cmd, "make", "clean");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, "make", "RAYLIB_SRC_PATH=.", "PLATFORM=PLATFORM_DESKTOP");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, "make", "clean");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, "make", "RAYLIB_SRC_PATH=.", "PLATFORM=PLATFORM_DESKTOP", "RAYLIB_LIBTYPE=SHARED");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, "make", "clean");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, "make", "RAYLIB_SRC_PATH=.", "PLATFORM=PLATFORM_WEB");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  ASSERT(nob_set_current_dir(project_root_path));

  return 1;
}

int build_raylib_static(void) {
  nob_log(NOB_INFO, "building raylib static");

  Nob_Cmd cmd = {0};

  ASSERT(nob_set_current_dir("./third_party/raylib"));

  nob_cmd_append(&cmd, "make", "clean");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, "make", "RAYLIB_SRC_PATH=.", "PLATFORM=PLATFORM_DESKTOP");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  ASSERT(nob_set_current_dir(project_root_path));

  return 1;
}

int build_raylib_shared(void) {
  nob_log(NOB_INFO, "building raylib shared");

  Nob_Cmd cmd = {0};

  ASSERT(nob_set_current_dir("./third_party/raylib"));

  nob_cmd_append(&cmd, "make", "clean");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, "make", "RAYLIB_SRC_PATH=.", "PLATFORM=PLATFORM_DESKTOP", "RAYLIB_LIBTYPE=SHARED");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  ASSERT(nob_set_current_dir(project_root_path));

  return 1;
}

int build_raylib_web(void) {
  nob_log(NOB_INFO, "building raylib web");

  Nob_Cmd cmd = {0};

  ASSERT(nob_set_current_dir("./third_party/raylib"));

  nob_cmd_append(&cmd, "make", "clean");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, "make", "RAYLIB_SRC_PATH=.", "PLATFORM=PLATFORM_WEB");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  ASSERT(nob_set_current_dir(project_root_path));

  return 1;
}

int build_metaprogram(void) {
  nob_log(NOB_INFO, "building metaprogram");

  run_tags();

  Nob_Cmd cmd = {0};
  nob_cmd_append(&cmd, CC, DEV_FLAGS, "-fPIC", "metaprogram.c", "-o", METAPROGRAM_EXE, RAYLIB_STATIC_LINK_OPTIONS, STATIC_BUILD_LDFLAGS);

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

  nob_cmd_append(&cmd, CC, DEV_FLAGS, "static_main.c", RAYLIB_STATIC_LINK_OPTIONS, "-o", EXE, STATIC_BUILD_LDFLAGS);
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}

int build_wasm(void) {
  run_metaprogram();
  run_tags();

  Nob_Cmd cmd = {0};

  nob_log(NOB_INFO, "building for WASM");

  ASSERT(nob_mkdir_if_not_exists("build"));
  ASSERT(nob_mkdir_if_not_exists("./build/wasm"));

  char *target = "wasm_main.c";
  nob_cmd_append(&cmd, "emcc", WASM_FLAGS, "--preload-file", "./aseprite/atlas.png", "--preload-file", "./sprites/nightsky.png", "--preload-file", "./sounds/", target, RAYLIB_STATIC_LINK_WASM_OPTIONS, RAYLIB_STATIC_LINK_WASM_OPTIONS, "-sEXPORTED_RUNTIME_METHODS=ccall", "-sUSE_GLFW=3", "-sFORCE_FILESYSTEM=1", "-sMODULARIZE=1", "-sWASM_WORKERS=1", "-sAUDIO_WORKLET=1", "-sUSE_PTHREADS=1", "-sWASM=1", "-sEXPORT_ES6=1", "-sGL_ENABLE_GET_PROC_ADDRESS", "-sINVOKE_RUN=0", "-sNO_EXIT_RUNTIME=1", "-sMINIFY_HTML=0", "-o", "./build/wasm/invaders.js");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}

int run_tags(void) {
  Nob_Cmd cmd = {0};

  nob_cmd_append(&cmd, CTAGS, "-w", "--sort=yes", "--language-force=c", "--c-kinds=+zfx", "--extras=+q", "--fields=+n", "--exclude='*.h'", "--exclude=third_party", "-R");
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  nob_cmd_append(&cmd, CTAGS, "-w", "--sort=yes", "--language-force=c", "--c-kinds=+zpx", "--extras=+q", "--fields=+n", "-a", "--recurse", RAYLIB_HEADERS);
  if(!nob_cmd_run_sync_and_reset(&cmd)) return 0;

  return 1;
}


#ifdef OS_WINDOWS
#error "windows support not implemented"
#elif defined(OS_MAC)

#define VIM_PROJECT_FILE \
  "let project_root = getcwd()\n"\
  "let project_build = project_root . '/nob'\n"\
  "let project_exe = '/invaders'\n"\
  "let project_run = project_root . project_exe\n"\
  "let project_debug = 'open -a Visual\\ Studio\\ Code ' . project_root\n"\
  "\n" \
  "let &makeprg = project_build\n"\
  "\n"\
  "nnoremap <F7> :call jobstart('open -a Terminal ' . project_root, { 'detach':v:true })<CR>\n"\
  "nnoremap <F8> :call chdir(project_root)<CR>\n"\
  "nnoremap <F9> :wa<CR>:make<CR>\n"\
  "nnoremap <F10> :call jobstart(project_run, { 'detach':v:true })<CR>\n"\
  "nnoremap <F11> :call jobstart(project_debug, { 'detach':v:true })<CR>\n"\

#elif defined(OS_LINUX)

#define VIM_PROJECT_FILE \
  "let project_root = getcwd()\n"\
  "let project_build = project_root . '/nob'\n"\
  "let project_exe = '/invaders'\n"\
  "let project_run = project_root . project_exe\n"\
  "let project_debug = 'gf2 ' . project_root . project_exe\n"\
  "\n"\
  "let &makeprg = project_build\n"\
  "\n"\
  "nnoremap <F7> :call jobstart('alacritty --working-directory ' . project_root, { 'detach':v:true })<CR>\n"\
  "nnoremap <F8> :call chdir(project_root)<CR>\n"\
  "nnoremap <F9> :wa<CR>:make<CR>\n"\
  "nnoremap <F10> :call jobstart(project_run, { 'detach':v:true })<CR>\n"\
  "nnoremap <F11> :call jobstart(project_debug, { 'detach':v:true })<CR>\n"\

#else
#error "unsupported operating system"
#endif

char project_file[] = VIM_PROJECT_FILE;

int generate_vim_project_file(void) {
  Str8 path_str = push_str8f(scratch, "%s/.project.vim", project_root_path);
  nob_write_entire_file((char*)path_str.s, project_file, memory_strlen(project_file));
  return 1;
}


int main(int argc, char **argv) {

  { /* set project_root_path from argv[0] */
    int exe_name_len = memory_strlen(argv[0]);
    int i;
    for(i = 0; i < exe_name_len - STRLEN("/nob"); i++) {
      project_root_path[i] = argv[0][i];
    }
    project_root_path[i] = 0;
  }

  nob_set_current_dir(project_root_path);

  NOB_GO_REBUILD_URSELF(argc, argv);

  scratch = arena_alloc();

  //if(!generate_vim_project_file()) return 1;
  //if(!build_raylib_web()) return 1;
  //if(!build_raylib_static()) return 1;
  //if(!build_raylib_shared()) return 1;
  //if(!build_metaprogram()) return 1;
  //if(!build_wasm()) return 1;
  //if(!build_hot_reload()) return 1;
  if(!build_static()) return 1;

  return 0;
}
