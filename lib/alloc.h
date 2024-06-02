#ifndef _ALLOC_H_
#define _ALLOC_H_

#include "types.h"

#define PROT_N   0
#define PROT_R   (1 << 0)
#define PROT_W   (1 << 1)
#define PROT_X   (1 << 2)
#define PROT_RW  (PROT_R|PROT_W)
#define PROT_WX  (PROT_W|PROT_X)
#define PROT_RX  (PROT_R|PROT_X)
#define PROT_RWX (PROT_R|PROT_W|PROT_X)

typedef Mem32 Prot;

Ptr valloc__Size__Prot (Size, Prot);
void devalloc__rPtr__Size (rPtr, Size);

Ptr alloc__Size (Size);
Ptr realloc__rPtr__Size (rPtr, Size);
void dealloc__rPtr (rPtr);

#endif
