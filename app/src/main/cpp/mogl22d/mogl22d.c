/// whooo. es scheint zu funktionieren.

//////// ok, versuchen wir mal ein rechteck zu zeichnen
/// wir machens richtig doof:
/// wir zeichnen zwei dreiecke und legen eine textur drauf

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <EGL/egl.h>
#include <GLES/gl.h>

#include "mogl22d.h"

#include <android/log.h>

#include <gc.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))


void mogl22d_segfault(void)
{
  // stop the program cold
  LOGI("SEGFAULTING TO STOP PROGRAM\n");
  //*((int *)NULL) = 1337;
  __builtin_trap();
}

void handle_error(void)
{
  GLenum err;
  int there_was_an_error = 0;
  while((err = glGetError()) != GL_NO_ERROR) {
    there_was_an_error = 1;

    switch (err) {
    case GL_INVALID_ENUM:
      LOGI("GL_INVALID_ENUM");
      break;
    case GL_INVALID_VALUE:
      LOGI("GL_INVALID_VALUE");
      break;
    case GL_INVALID_OPERATION:
      LOGI("GL_INVALID_OPERATION");
      break;
    case GL_STACK_OVERFLOW:
      LOGI("GL_STACK_OVERFLOW");
      break;
    case GL_STACK_UNDERFLOW:
      LOGI("GL_STACK_UNDERFLOW");
      break;
    case GL_OUT_OF_MEMORY:
      LOGI("GL_OUT_OF_MEMORY");
      break;
    }
  }
  if (there_was_an_error)
    mogl22d_segfault();
}

static const GLfloat g_vertex_buffer_data[] = {       // triangle
  -1.0f, -1.0f, 0.0f,
  1.0f, -1.0f, 0.0f,
  -1.0f,  1.0f, 0.0f,

  1.0f, 1.0f, 0.0f,
  -1.0f, 1.0f, 0.0f,
  1.0f,  -1.0f, 0.0f,
};

// TODO kann spaeter weg
static const GLfloat g_color_buffer_data[] = {       // triangle
  0.0f, 0.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 0.0f, 0.0f,
  0.0f,  0.0f, 0.0f, 0.0f,

  1.0f, 1.0f, 0.5f, 0.0f,
  0.0f, 1.0f, 0.5f, 0.0f,
  1.0f,  0.0f, 0.5f, 0.0f,
};
// !TODO

static const GLfloat g_uv_buffer_data[] = {
  0.0f, 0.0f,
  1.0f, 0.0f,
  0.0f,  1.0f,

  1.0f, 1.0f,
  0.0f, 1.0f,
  1.0f, 0.0f,
};


mogl22d_surface *mogl22d_init(int width, int height)
{
  mogl22d_surface *ms;

  ms = (mogl22d_surface *)GC_malloc(sizeof (mogl22d_surface));
  ms->imagedata = (char *)GC_malloc(width * height * 3);   // rgb
  ms->width  = width;
  ms->height = height;

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  /* glEnableClientState(GL_COLOR_ARRAY); */

  handle_error();
  glEnable(GL_TEXTURE_2D);

  GLuint VertexArrayID;
  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);   // dark blue /// don't need this in init?

  
  glClientActiveTexture(GL_TEXTURE0);   // hm?
  glActiveTexture(GL_TEXTURE0);
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  GLuint textureID;
  glGenTextures(1, &textureID);
  // "Bind" the newly created texture : all future texture functions will modify this texture
  glBindTexture(GL_TEXTURE_2D, textureID);
  // Give the image to OpenGL
  LOGI("generating texture %i x %i\n", ms->width, ms->height);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ms->width, ms->height, 0, GL_RGB, GL_UNSIGNED_BYTE, ms->imagedata);
  handle_error();
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  return ms;
}

void mogl22d_flip(mogl22d_surface *ms)
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ms->width, ms->height, 0, GL_RGB, GL_UNSIGNED_BYTE, ms->imagedata);



    glVertexPointer(3, GL_FLOAT, 0, g_vertex_buffer_data);
    /* glColorPointer(4, GL_FLOAT, 0, g_color_buffer_data); */
    glTexCoordPointer(2, GL_FLOAT, 0, g_uv_buffer_data);
    /* glDrawElements(GL_TRIANGLES, 2, GL_UNSIGNED_BYTE, indices); */
    glDrawArrays(GL_TRIANGLES, 0, 6);

    handle_error();
    //glDisableVertexAttribArray(0);
}

void mogl22d_draw_point(mogl22d_surface *ms, unsigned x, unsigned y, unsigned char r, unsigned char g, unsigned char b)
{
  *(ms->imagedata + ms->width*3*y + 3*x + 0) = b;
  *(ms->imagedata + ms->width*3*y + 3*x + 1) = g;
  *(ms->imagedata + ms->width*3*y + 3*x + 2) = r;
}

void mogl22d_draw_x(mogl22d_surface *ms)
{
  int x, y;

  for (x=0; x<ms->width; ++x) {
    mogl22d_draw_point(ms, x, x * ms->height / ms->width, 255, 0, 0);   // rot von unten links
    mogl22d_draw_point(ms, x, ms->height - (x * ms->height / ms->width), 0, 255, 0);    // gruen von oben links
  }

  /* for (x=0; x<ms->width * ms->height * 3; ++x) */
  /*   ms->imagedata[x] = x%255; */
}

void mogl22d_clear(mogl22d_surface *ms)
{
  memset(ms->imagedata, 0, ms->height * ms->width * 3);
}
