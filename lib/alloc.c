
#include "alloc.h"

#define MEM_FLAG 0xF
#define MEM_FREE (1 << 0)
#define MEM_STRT (1 << 1)

typedef struct _MemBlk_s MemBlk;
typedef struct _MemBlk_s* rMemBlk;
typedef struct _ObjHdr_s ObjHdr;

strcut _ObjHdr_s {
  Size size;
  Type type;
  Mem32 ofst;
};

struct _MemBlk_s {
  rMemBlk prev;
  rMemBlk next;
  ObjHdr  obj;
};

  Ptr
valloc (size, prot)
  Size size;
  Prot prot;
{
  register void* ptr asm ("rax");

  size = (size + 0xFFF) & ~0xFFF;
  asm (
    "xorq %%rdi, %%rdi\n\t"
    "movl %0,    %%esi\n\t"
    "movl %1,    %%edx\n\t"
    "movl $34,   %%r10d\n\t"
    "movl $-1,   %%r8d\n\t"
    "xorl %%r9d, %%r9d\n\t"
    "movq $9,    %%rax\n\t"
    "syscall"
    :
    : "r" (size), "r" (prot)
    : "%rdi", "%rsi", "%rdx",
      "%r10", "%r8",  "%r9"
  );
  if (ptr == (void*) -1) {
    ptr = 0;
  }
  return ptr;
}

  void
devalloc (ptr, size)
  rPtr ptr;
  Size size;
{
  if (ptr && *ptr) {
    asm (
      "movq %0,  %%rdi\n\t"
      "movl %1,  %%esi\n\t"
      "movq $11, %%rax\n\t"
      "syscall"
      :
      : "r" (*ptr), "r" (size)
    );
    *ptr = 0;
  }
  return;
}

static Memory* last;
static Memory* first;

  Ptr
alloc (size)
  Size size;
{
  Memory* cur = 0;
  Mem32 actsize = 0;
  Mem32 ofst = 0;

  size += 16;
  size = (size + 0xF) & ~0xF;
  
  cur = first;
loop:
  if (!cur) {
    actsize = (size + 0xFFFFF) & ~0xFFFFF;
    cur = valloc (actsize, PROT_RW);
    if (!cur) {
      return 0;
    }
    cur->size = actsize | MEM_FREE | MEM_STRT;
    cur->next = 0;
    if (last) {
      cur->prev = last - cur;
      last->next = last - cur;
    }
    else {
      cur->prev = 0;
      last = cur;
      first = cur;
    }
  }
  if (!(cur->size & MEM_FREE) ||
      (cur->size & ~MEM_FLAG) < size) {
    if (cur->next) {
      cur += cur->next;
    }
    else {
      cur = 0;
    }
    goto loop;
  }
  if ((cur->size & ~MEM_FLAG) > size) {
    actsize = (cur->size & ~MEM_FLAG) - size;
    ofst = size >> 4;
    cur[ofst].prev = cur - (cur + ofst);
    if (cur->next) {
      cur[ofst].next = (cur + ofst) - (cur + cur->next);
    }
    cur[ofst].size = actsize | MEM_FREE;
    cur->next = (cur + ofst) - cur;
    cur->size = size;
  }
  cur->type = type;
  return (cur + 1);      
}
  
  void
dealloc (ptr)
  rPtr ptr;
{
  Memory* cur = 0;

  if (ptr && *ptr) {
    cur = ((Memory*)*ptr) - 1;
    if (cur->size & MEM_FREE) {
      *ptr = 0;
      return;
    }
    if (cur->next &&
        cur[cur->next].size & MEM_FREE &&
        !(cur[cur->next].size & MEM_STRT)) {
      cur->next = (cur) - (cur + cur->next + cur[cur->next].next);
      cur->size += cur[cur->next].size & ~MEM_FLAG;
    }
    if (cur->prev && 
        cur[cur->prev].size & MEM_FREE) {
      cur[cur->prev].next = (cur + cur->prev) - (cur + cur->next);
      cur[cur->prev].size += cur->size & ~MEM_FLAG;
    }
    cur->size |= MEM_FREE;
    if (cur->size & MEM_STRT) {
      devalloc (cur, cur->size & ~MEM_FLAG);
    }
    *ptr = 0;
  }
  return;
}

  Ptr
realloc (ptr, size)
  rPtr ptr;
  Size size;
{
  if (!ptr || !*ptr) {
    return malloc (size);
  }
  return 0;
}

