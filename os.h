#ifndef JLIB_OS_H
#define JLIB_OS_H


#include "basic.h"


// NOTE these wrap malloc and free
// in future we'll do a virtual allocation scheme
void* os_alloc(u64 size);
void  os_free(void *ptr);


#endif


#if defined(JLIB_OS_IMPL) != defined(_UNITY_BUILD_)

#ifdef _UNITY_BUILD_
#define JLIB_OS_IMPL
#endif

#if defined(OS_LINUX) || defined(OS_MAC) || defined(OS_WEB)



void* os_alloc(u64 size) {
  return malloc(size);
}

void os_free(void *ptr) {
  free(ptr);
}



#elif defined(OS_WINDOWS)

#error "windows support not implemented"

#else

#error "unsupported operating system"

#endif


#endif
