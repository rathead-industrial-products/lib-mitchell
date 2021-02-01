
/*********************************************************************
*
* Mock for SEGGER_RTT_Write
*
*********************************************************************/
#include <stdio.h>
unsigned SEGGER_RTT_Write(unsigned BufferIndex, const void* pBuffer, unsigned NumBytes) {

    printf(pBuffer);
  return (0);
}

