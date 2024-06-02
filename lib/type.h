#ifndef _TYPES_H_
#define _TYPES_H_

typedef   signed char  Int8;
typedef   signed short Int16; 
typedef   signed int   Int32; 
typedef   signed long  Int64;
typedef unsigned char  Mem8; 
typedef unsigned short Mem16; 
typedef unsigned int   Mem32; 
typedef unsigned long  Mem64;

typedef void* Ptr;
typedef Ptr* vPtr;
typedef Mem32 Size;
typedef Mem32 Type;
typedef Mem32 Flag;
typedef void Void;

struct _Type_s {
  vPtr vtbl;
  Size size;
  Flag flag;
};

#endif
