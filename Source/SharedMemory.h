#ifndef __SHARED_MEMORY_H
#define __SHARED_MEMORY_H

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

   typedef enum { 
     AUDIO_IDLE_STATUS = 0, 
     AUDIO_READY_STATUS, 
     AUDIO_PROCESSED_STATUS, 
     AUDIO_EXIT_STATUS, 
     AUDIO_ERROR_STATUS 
   } SharedMemoryAudioStatus;

   typedef struct {
     uint8_t checksum;
     SharedMemoryAudioStatus status;
     int16_t* audio_input;
     int16_t* audio_output;
     uint8_t audio_bitdepth;
     uint16_t audio_blocksize;
     uint32_t audio_samplingrate;
     uint16_t patch_selected_id;
     uint8_t patch_mode;
     uint16_t* parameters;
     uint8_t parameters_size;
     uint16_t buttons;
     int8_t error;
     void (*registerPatch)(const char* name, uint8_t inputChannels, uint8_t outputChannels);
     void (*registerPatchParameter)(uint8_t id, const char* name);
     void (*exitProgram)();
     uint32_t cycles_per_block;
     uint32_t heap_bytes_used;
   } SharedMemory;

#define CHECKSUM_ERROR_STATUS      -10
#define OUT_OF_MEMORY_ERROR_STATUS -20

extern volatile SharedMemory smem;

#ifdef __cplusplus
}
#endif

#endif /* __SHARED_MEMORY_H */
