#include <stdlib.h>
#include <string.h>
#include "myarraystream.h"

MYSTREAM *myopen(void *arr,  int length)
{
  MYSTREAM *f;
  f=(MYSTREAM *)malloc(sizeof (MYSTREAM));
  f->length = length;
  f->startptr = (char *)arr;
  f->pos = 0;
  return f;
}

int myread(char *dest, int n, MYSTREAM *f)
{
  if (n > (f->length - f->pos))
    n = f->length - f->pos;

  memcpy(dest, f->startptr + f->pos, n);
  f->pos += n;
  return n;
}

void myclose(MYSTREAM *f)
{
  free(f);
}
