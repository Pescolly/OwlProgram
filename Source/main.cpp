#define DEBUG_DWT /* now done by firmware */
#define DEBUG_MEM
// #define STARTUP_CODE

#include <string.h>
#include <inttypes.h>
#include "SharedMemory.h"
#include "owlcontrol.h"
#include "stm32f4xx.h"
#ifdef DEBUG_MEM
#include <malloc.h>
#else
extern "C" void* _sbrk(int incr);
#endif /* DEBUG_MEM */
extern void setup();
extern void processBlock();

extern char _sbss[];
extern char _ebss[];
// extern char _sidata[];
// extern char _sdata[];
// extern char _edata[];

extern "C" void __libc_init_array();

// extern "C" {
//   SharedMemory ProgramVector;
// }
int doneFft=0;
#define BANK1_SRAM3 0x68000000
int main(void){
#ifdef STARTUP_CODE
  /* from startup.s Reset_Handler:
     zero-fill _ebss
     Call static constructors:   bl __libc_init_array
     call C++ static initializers?
  */
  memset(_sbss, 0, _ebss-_sbss); // zero fill the BSS segment
  // memcpy(_sidata, _sdata, _sdata-_edata); // Copy the data segment initializers
  __libc_init_array(); // caused reset when .data had been strippped
#endif /* STARTUP_CODE */

#ifdef DEBUG_DWT
  volatile unsigned int *DWT_CYCCNT = (volatile unsigned int *)0xE0001004; //address of the register
  volatile unsigned int *DWT_CONTROL = (volatile unsigned int *)0xE0001000; //address of the register
  volatile unsigned int *SCB_DEMCR = (volatile unsigned int *)0xE000EDFC; //address of the register
  *SCB_DEMCR = *SCB_DEMCR | 0x01000000;
  *DWT_CONTROL = *DWT_CONTROL | 1 ; // enable the counter
  // if(!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
  //   CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  //   DWT->CYCCNT = 0;
  //   DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  // }
#endif /* DEBUG_DWT */

  if(getSharedMemory()->checksum != sizeof(SharedMemory)){
    // problem!
    // getSharedMemory()->status = AUDIO_ERROR_STATUS;
    getSharedMemory()->error = CHECKSUM_ERROR_STATUS;
    getSharedMemory()->message = "ProgramVector checksum error";
    getSharedMemory()->programStatus(AUDIO_ERROR_STATUS);
    // getSharedMemory()->exitProgram();
    // return -1;
  }

  setup();

#ifdef DEBUG_MEM
  struct mallinfo minfo = mallinfo(); // never returns when -O1 or -O2 ?
  /* ^ may cause OwlWare.sysex to trip to:
   USART6_IRQHandler () at ./Source/startup.s:142
   142	  b  Infinite_Loop */
  // getSharedMemory()->heap_bytes_used = minfo.uordblks;
  getSharedMemory()->heap_bytes_used = minfo.arena;
#else
  // getSharedMemory()->heap_bytes_used = (int32_t)_sbrk(0) - (int32_t)0x68000000;
#endif /* DEBUG_MEM */

  for(;;){
    getSharedMemory()->programReady();
    // if(getSharedMemory()->status == AUDIO_READY_STATUS){
#ifdef DEBUG_DWT
      *DWT_CYCCNT = 0; // reset the counter
      // DWT->CYCCNT = 0; // reset the counter
#endif /* DEBUG_DWT */
    doneFft=0;
      processBlock();
    static int dasCount=0;
    dasCount++;
    static int maxCount=0;
    static int minCount=888888888;
    static int wasfft=0;
    if(maxCount<*DWT_CYCCNT/128){
      maxCount=*DWT_CYCCNT/128;
      wasfft=doneFft;
    }
    if(minCount>*DWT_CYCCNT/128){
      minCount=*DWT_CYCCNT/128;
      wasfft=doneFft;
    }
    if((dasCount&127)==0){
      debugMessage("minCount, maxCount", minCount, maxCount);
      maxCount=0;
      minCount=88888888;
      wasfft=-1;
    }
#ifdef DEBUG_DWT
      getSharedMemory()->cycles_per_block = *DWT_CYCCNT;
      // dwt_count = DWT->CYCCNT;
#endif /* DEBUG_DWT */
      // if(getSharedMemory()->status == AUDIO_EXIT_STATUS)
      // 	break;
      // getSharedMemory()->status = AUDIO_PROCESSED_STATUS; // always check status before changing it
    // }
    // if(getSharedMemory()->status == AUDIO_EXIT_STATUS)
    //   break;
  }
}
