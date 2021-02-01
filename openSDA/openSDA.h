/*****************************************************************************

    openSDA.h - Support for debugging using openSDA.

    (c) Copyright 2016 DDPA LLC
    ALL RIGHTS RESERVED.

 *****************************************************************************/

#ifndef _init_H_
#define _init_H_

#include  <stdint.h>

void  openSDAInit(void);
void  openSDAWrite(char *s);
bool  openSDAPut(char c);
bool  openSDAGet(char *c);



#endif  /* _init_H_ */
