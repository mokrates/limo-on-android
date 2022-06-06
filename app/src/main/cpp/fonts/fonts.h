/* Header fuer libsimplefonts.a */

#ifdef __cplusplus
extern "C" {
#endif


#include <stddef.h>
#include <stdio.h>

#define F_TRANSPARENT   0xff

struct FontChar {
  int width;
  unsigned *bitmap;
};

typedef struct FONT {
  struct FontChar pics[256];
  int height;
} *font;

font FontLoad(char *);
font FontLoadFromArray(unsigned char *array, unsigned int size);
void FontDestroy(font);

void FontPutC(font, char);
void FontWrite(font, char *);
void FontWritef(font, char *, ...);
void FontNewLine(void);

void FontSetSize(int);
int  FontGetSize(void);

void FontSetColor(int, int);
int  FontGetBColor(void);
int  FontGetColor(void);

void FontGotoXY(int, int);
int  FontGetX(void);
int  FontGetY(void);

extern unsigned char dosbox_8x16_fnt[];
extern unsigned int dosbox_8x16_fnt_len;

extern void (*FontPlotFunction)(int x, int y, int c);
extern void (*FontFilledBoxFunction)(int x, int y, int x2, int y2, int c);

#ifdef __cplusplus
}
#endif
