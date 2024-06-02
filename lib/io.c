
#include "io.h"

  Int64
write__Int32__rMem8__Size (fd, bf, sz)
  Int32 fd;
  rMem8 bf;
  Size sz;
{
  register Int64 res asm ("rax");
  asm (
    "movq %2, %%rdx\n\t"
    "movq %1, %%rsi\n\t"
    "movl %0, %%edi\n\t"    
    "movq $1, %%rax\n\t"
    "syscall"
    :
    : "r" (fd),
      "r" (bf),
      "r" (sz)
    : "%rdi", "%rsi", "%rdx"
  );
  if (res == -1) {
    res = 0;
  }
  return res;
}

  Void
print__rStr (str)
  rStr str;
{
  write__Int32__rMem8__Mem64
    (OUTFD, str, size__rStr(str));
}
 
