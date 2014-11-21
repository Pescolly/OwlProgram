#include <stdlib.h>
#include "myalloc.h"

extern "C" void *__gxx_personality_v0;
extern "C" void __cxa_end_cleanup (void);
extern "C" void __cxa_pure_virtual(){}

void * operator new(size_t size) { return myalloc(size); }
void * operator new (size_t, void * p) { return p ; }
void * operator new[](size_t size) { return myalloc(size); }
void operator delete(void* ptr) { myfree(ptr); }
void operator delete[](void * ptr) { myfree(ptr); }
