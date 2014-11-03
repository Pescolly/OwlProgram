#ifndef __PatchRegistry_h__
#define __PatchRegistry_h__

#include <string>
#include "StompBox.h"
#include "device.h"

class PatchRegistry;
extern PatchRegistry registry;

class PatchRegistry {
  typedef Patch* (*PatchCreator)(); // function pointer to create Patch
public:
  PatchRegistry();
  const char* getName(unsigned int index); 
  Patch* create(unsigned int index);
  Patch* create(const char* name);
  unsigned int getNumberOfPatches();
  void registerPatch(const char* name, uint8_t inputs, uint8_t outputs, 
		     PatchCreator creator);
private:
  template<class T> struct Register {
    static Patch* construct() {
      return new T();
    };
    static PatchCreator* creator;
  };
private:
  const char* names[MAX_NUMBER_OF_PATCHES];
  PatchCreator creators[MAX_NUMBER_OF_PATCHES];
  int nofPatches;
};

#endif // __PatchRegistry_h__
