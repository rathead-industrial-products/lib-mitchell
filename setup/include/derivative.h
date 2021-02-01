
/*
 * Define the hardware target by the defined preprocessor macro.
 * This is used to pull in the proper headers for the specific
 * processor derivative being targeted.
 */


#ifndef _derivative_H_
#define _derivative_H_


#if (defined(CPU_MKL03Z32CAF4) || defined(CPU_MKL03Z8VFG4) || defined(CPU_MKL03Z16VFG4) || \
    defined(CPU_MKL03Z32VFG4)  || defined(CPU_MKL03Z8VFK4) || defined(CPU_MKL03Z16VFK4) || \
    defined(CPU_MKL03Z32VFK4))
    #include "MKL03Z4.h"

#elif (defined(CPU_MKL25Z32VFM4) || defined(CPU_MKL25Z64VFM4) || defined(CPU_MKL25Z128VFM4) || \
    defined(CPU_MKL25Z32VFT4)    || defined(CPU_MKL25Z64VFT4) || defined(CPU_MKL25Z128VFT4) || \
    defined(CPU_MKL25Z32VLH4)    || defined(CPU_MKL25Z64VLH4) || defined(CPU_MKL25Z128VLH4) || \
    defined(CPU_MKL25Z32VLK4)    || defined(CPU_MKL25Z64VLK4) || defined(CPU_MKL25Z128VLK4))
    #include "MKL25Z4.h"

#elif (defined(CPU_MKL43Z128VLH4) || defined(CPU_MKL43Z256VLH4) || defined(CPU_MKL43Z128VMP4) || \
    defined(CPU_MKL43Z256VMP4))
    #include "MKL43Z4.h"

#elif (defined(CPU_MK22FN256CAH12) || defined(CPU_MK22FN128CAH12) || defined(CPU_MK22FN256VDC12) || \
    defined(CPU_MK22FN256VLH12)    || defined(CPU_MK22FN256VLL12) || defined(CPU_MK22FN256VMP12))
    #include "MK22F25612.h"

#else
    #error "No valid CPU defined!"
#endif


#endif  /* _derivative_H_ */

