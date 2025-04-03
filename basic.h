#ifndef JLIB_BASIC_H
#define JLIB_BASIC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define SQUARE(x) (x*x)
#define TIMES2(x) (x+x)
#define HALF(x) (x*0.5f)
#define IS_POW_2(x) ((x & (x-1)) == 0)
#define SIGN_EXTEND_S64(x, n) (S64)((n >= 64) ? (S64)x : (S64)((S64)x | (S64)(-((S64)x >> ((S64)n - 1lu)) << (S64)n)))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define Arrlen(x) (sizeof(x)/sizeof(*x))
#define STRLEN(x) ((sizeof(x)/sizeof(*x))-1)
#define Arr(T) T *
#define Map(K, V) struct { K key; V value; } *
#define Dict(V) struct { char *key; V value; } *
#define UNREACHABLE assert(!"UNREACHABLE")
#define UNIMPLEMENTED assert(!"UNIMPLEMENTED")
#define GLUE_(a,b) a##b
#define GLUE(a,b) GLUE_(a,b)
#define STATIC_ASSERT(expr, id) U8 GLUE(id, __LINE__)[(expr)?1:-1]
#define PANIC(msg) assert(!msg)
#define PASS assert(1)
#define NOOP (0+0)
#define DUNNO fprintf(stderr, "======\nDUNNO WHAT HAPPENS ON LINE %i IN %s()\n======\n", __LINE__, __func__)
#define INLINE __attribute__((always_inline)) inline
#if defined(stbsp_sprintf) && defined(stbsp_snprintf)
#undef sprintf
#define sprintf stbsp_sprintf
#undef snprintf
#define snprintf stbsp_snprintf
#endif
#define VEC2_IHAT ((Vector2){1.0f, 0.0f})
#define VEC2_JHAT ((Vector2){0.0f, 1.0f})
#define VEC2_ORIGIN ((Vector2){0.0f,0.0f})
#define VEC2_ZERO ((Vector2){0.0f,0.0f})
#define F32_NEGATIVE_ZERO (u32)(0x80000000)
#define member_size(type, member) sizeof(((type*)0)->member)

typedef int64_t  S64;
typedef uint64_t U64;
typedef int32_t  S32;
typedef uint32_t U32;
typedef int16_t  S16;
typedef uint16_t U16;
typedef int8_t   S8;
typedef uint8_t  U8;
typedef float    F32;
typedef double   F64;

typedef bool     B8;
typedef uint16_t B16;
typedef uint32_t B32;
typedef uint64_t B64;


/* string type */

typedef struct Str8 Str8;


struct Str8 {
  U8 *d;
  S64 len;
};


#define str8_lit(s) ((Str8){ .d = (U8*)(s), .len = sizeof(s) - 1 })

#endif
