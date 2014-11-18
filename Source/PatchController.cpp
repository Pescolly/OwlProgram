#include "PatchController.h"
#include "owlcontrol.h"
// #include "CodecController.h"
#include "MemoryBuffer.hpp"
#include "SharedMemory.h"
#include "ApplicationSettings.h"

#define SINGLE_MODE          1
#define DUAL_GREEN_MODE      2
#define DUAL_RED_MODE        3
#define SERIES_GREEN_MODE    4
#define SERIES_RED_MODE      5
#define PARALLEL_GREEN_MODE  6
#define PARALLEL_RED_MODE    7

PatchController::PatchController(){
}

PatchController::~PatchController(){
}

void PatchController::setParameterValues(uint16_t* values){
  parameterValues = values;
}

void PatchController::init(){
  parameterValues = smem.parameters;
  setActiveSlot(GREEN);
  initialisePatch(GREEN);
  initialisePatch(RED);
}

void PatchController::reset(){
  init();
}

// __attribute__ ((section (".coderam")))
void PatchController::processParallel(AudioBuffer& buffer){
  MemoryBuffer left(buffer.getSamples(0), 1, buffer.getSize());
  MemoryBuffer right(buffer.getSamples(1), 1, buffer.getSize());
  green.patch->processAudio(left);
  red.patch->processAudio(right);
}

// __attribute__ ((section (".coderam")))
void PatchController::initialisePatch(LedPin slot){
  // the initialisingProcessor must be set
  // so that it can be picked up by a call to getInitialisingProcessor() from the Patch constructor
  if(slot == RED){
    initialisingProcessor = &red;
    red.setPatch(settings.patch_red);
  }else{
    initialisingProcessor = &green;
    green.setPatch(settings.patch_green);
  }
}

PatchProcessor* PatchController::getInitialisingPatchProcessor(){
  return initialisingProcessor;
}

// __attribute__ ((section (".coderam")))
void PatchController::process(AudioBuffer& buffer){
  if(activeSlot == GREEN && green.index != settings.patch_green){
    initialisePatch(GREEN);
    // codec.softMute(false);
    debugClear();
    return;
  }else if(activeSlot == RED && red.index != settings.patch_red){
    initialisePatch(RED);
    // codec.softMute(false);
    debugClear();
    return;
  }
  switch(mode){
  case SINGLE_MODE:
  case DUAL_GREEN_MODE:
    green.setParameterValues(parameterValues);
    green.patch->processAudio(buffer);
    break;
  case DUAL_RED_MODE:
    red.setParameterValues(parameterValues);
    red.patch->processAudio(buffer);
    break;
  case SERIES_GREEN_MODE:
    green.setParameterValues(parameterValues);
    green.patch->processAudio(buffer);
    red.patch->processAudio(buffer);
    break;
  case SERIES_RED_MODE:
    red.setParameterValues(parameterValues);
    green.patch->processAudio(buffer);
    red.patch->processAudio(buffer);
    break;
  case PARALLEL_GREEN_MODE:
    green.setParameterValues(parameterValues);
    processParallel(buffer);
    break;
  case PARALLEL_RED_MODE:
    red.setParameterValues(parameterValues);
    processParallel(buffer);
    break;
  }
}

void PatchController::setPatch(LedPin slot, uint8_t index){
  // codec.softMute(true);
  if(slot == RED){
    settings.patch_red = index;
  }else{
    settings.patch_green = index;
  }
  setActiveSlot(slot);
}

LedPin PatchController::getActiveSlot(){
  return activeSlot;
}

void PatchController::setActiveSlot(LedPin slot){
  switch(settings.patch_mode){
  case(PATCHMODE_SINGLE):
    mode = SINGLE_MODE;
    break;
  case(PATCHMODE_DUAL):
    mode = slot == RED ? DUAL_RED_MODE : DUAL_GREEN_MODE;
    break;
  case(PATCHMODE_SERIES):
    mode = slot == RED ? SERIES_RED_MODE : SERIES_GREEN_MODE;
    break;
  case(PATCHMODE_PARALLEL):
    mode = slot == RED ? PARALLEL_RED_MODE : PARALLEL_GREEN_MODE;
    break;
  }
  activeSlot = slot;
  setLed(slot);
}

void PatchController::toggleActiveSlot(){
  if(activeSlot == GREEN)
    setActiveSlot(RED);
  else
    setActiveSlot(GREEN);
}

PatchProcessor* PatchController::getActivePatchProcessor(){
  if(activeSlot == RED)
    return &red;
  return &green;
}
