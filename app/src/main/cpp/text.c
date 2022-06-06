#include <EGL/egl.h>
#include <GLES/gl.h>

#include "fonts/fonts.h"
#include "mogl22d/mogl22d.h"

mogl22d_surface *mogl22d_textms;

static unsigned int ct[16][3] = {
  {0, 0, 0},
  {0, 0, 168},
  {0, 168, 0},
  {0, 168, 168},
  {168, 0, 0},
  {168, 0, 168},
  {168, 168, 0},
  {208, 208, 208},
  {168, 168, 168},
  {0, 0, 252},
  {0, 252, 0},
  {0, 252, 252},
  {252, 0, 0},
  {252, 0, 252},
  {252, 252, 0},
  {252, 252, 252},
};

void mogl22d_font_plot_function(int x, int y, int c)
{
  mogl22d_draw_point(mogl22d_textms, x, mogl22d_textms->height - y, ct[c][0], ct[c][1], ct[c][2]);
}

void mogl22d_filled_box_function(int x1, int y1, int x2, int y2, int c)
{
  int i, j;
  for (i=y1; i<y2; ++i)
    for (j=x1; j<x2; ++j)
      mogl22d_draw_point(mogl22d_textms, j, mogl22d_textms->height - i - 1, ct[c][0], ct[c][1], ct[c][2]);;
}
