#ifdef __cplusplus
extern "C" {
#endif

  typedef struct _MOGL22D_SURFACE {
    int width;
    int height;
    char *imagedata;

    GLuint ProgramID;
    //GLuint vertexbuffer;
  } mogl22d_surface;

  // GL should be already bound and everything
  mogl22d_surface *mogl22d_init(int width, int height);
  void mogl22d_flip(mogl22d_surface *ms);
  void mogl22d_draw_point(mogl22d_surface *ms, unsigned x, unsigned y, unsigned char r, unsigned char g, unsigned char b);
  void mogl22d_draw_x(mogl22d_surface *ms);
  void mogl22d_clear(mogl22d_surface *ms);

#ifdef __cplusplus
}
#endif
