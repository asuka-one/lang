
#include "string.h"

const Type _Str_t = 0;

struct Str {
  vChr buf;
};

  Size
size__rStr (str)
  rSTr str;
{

}

  rStr
new__Str (void)
{
  return alloc (16, _Str_t);
}

  void
del__rrStr (str)
  rrStr str;
{
  dealloc (str);
}

  rStr
cat__rrStr__rStr (str1, str2)
  rrStr str1;
  rStr str2;
{

}

  rStr
cated__rStr__rStr (str1, str2)
  rStr str1;
  rStr str2;
{

}

