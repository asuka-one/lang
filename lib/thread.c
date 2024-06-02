
#include "thread.h"

  void
exit (code)
  Int32 code;
{
  asm (
    "movl %0,  %%edi\n\t"
    "movq $60, %%rax\n\t"
    "syscall"
    :
    : "r" (code)
    : "%rdi"
  );
}

