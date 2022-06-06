#include <stdlib.h>
#include <stdio.h>
#include "fonts.h"
#include "myarraystream.h"

font FontLoad_intern(MYSTREAM *f)
{
  font Font;
  int i, j;

  Font=(font)malloc(sizeof (struct FONT));
  if (Font==NULL)
    return NULL;

  myread(&Font->height, sizeof (int), f);

  for (i=0; i<256; ++i) {
    myread(&Font->pics[i].width, sizeof (int), f);
    Font->pics[i].bitmap =
      (unsigned *)calloc(Font->height, sizeof (unsigned));
    if (Font->pics[i].bitmap==NULL) {
        while (i-- > 0)
          free(Font->pics[i].bitmap);
        free(Font);
        return NULL;
    }
    myread(Font->pics[i].bitmap, sizeof (unsigned) * Font->height, f);
  }
  return Font;
}

font FontLoadFromArray(unsigned char *array, unsigned int size)
{
  MYSTREAM *f;

  if ((f=myopen(array, size)) == NULL)
    return NULL;
  else
    return FontLoad_intern(f);
}

/* font FontLoad(char *fname) */
/* { */
/*   FILE *f; */

/*   if ((f=fopen(fname, "rb")) == NULL) */
/*     return NULL; */
/*   else */
/*     return FontLoad_intern(f); */
/* } */
