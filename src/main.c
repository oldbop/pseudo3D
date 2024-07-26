#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifdef SYS_GL_HEADERS
#define GL_GLEXT_PROTOTYPES

#include <GL/gl.h>
#include <GL/glext.h>

#define LOAD_GL()

#else
#include <glad/gl.h>

#define LOAD_GL() gladLoadGL(glfwGetProcAddress)

#endif

#define TITLE  "pseudo3D"
#define WIDTH  960
#define HEIGHT 720
#define DSCALE 4
#define PI     3.141592653589793

#define COLOR(r, g, b, a) (r << 24) | (g << 16) | (b << 8)  | (a << 0)

typedef struct {
  int32_t dwidth, dheight, ox, oy, npixels;
  uint32_t *pixels;
} PixelArray;

PixelArray parr;

static const uint8_t map[] = {
  1, 1, 1, 1,
  1, 0, 0, 1,
  1, 0, 0, 1,
  1, 1, 1, 1
};

static inline void clear_screen(uint32_t color) {

  for (uint32_t i = 0; i < parr.npixels; ++i) {
    parr.pixels[i] = color;
  }
}

static inline void draw_vert_line(uint32_t color, int32_t x, int32_t len) {

  int32_t top = parr.oy + len / 2;
  int32_t bot = parr.oy - len / 2;

  for (uint32_t i = bot; i < top; ++i) {
    parr.pixels[i * parr.dwidth + x] = color;
  }
}

int main(int argc, char **argv) {

  if (!glfwInit()) {
    printf("GLFW: failed to initialise\n");
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, 0);

  GLFWwindow *win = glfwCreateWindow(WIDTH, HEIGHT, TITLE, NULL, NULL);

  if (!win) {
    printf("GLFW: failed to create window\n");
    return -1;
  }

  parr.dwidth  = WIDTH / DSCALE;
  parr.dheight = HEIGHT / DSCALE;
  parr.ox = parr.dwidth / 2;
  parr.oy = parr.dheight / 2;
  parr.npixels = parr.dwidth * parr.dheight;

  parr.pixels = malloc(parr.npixels * sizeof(uint32_t));

  if (!parr.pixels) {
    printf("PixelArray allocation failed\n");
    return -1;
  }

  glfwMakeContextCurrent(win);
  glfwSwapInterval(1);

  LOAD_GL();

  glViewport(0, 0, WIDTH, HEIGHT);

  uint32_t ftex;
  glGenTextures(1, &ftex);
  glBindTexture(GL_TEXTURE_2D, ftex);

  // Consider using GL_BGRA and GL_UNSIGNED_INT_8_8_8_8_REV
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, parr.dwidth, parr.dheight, 0, GL_RGBA,
               GL_UNSIGNED_INT_8_8_8_8, NULL);

  uint32_t fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);

  glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, ftex, 0);

  while (!glfwWindowShouldClose(win)) {

    double time = glfwGetTime();
    double length = ((sin(time) + 1.0) * parr.oy) + 0.1;

    clear_screen(COLOR(0x1a, 0x1a, 0x1a, 0xff));
    draw_vert_line(COLOR(0xff, 0xff, 0xff, 0xff), parr.ox, length);

    // Is there another way of doing this?
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, parr.dwidth, parr.dheight, GL_RGBA,
                    GL_UNSIGNED_INT_8_8_8_8, parr.pixels);

    // Is this optimal?
    glBlitFramebuffer(0, 0, parr.dwidth, parr.dheight, 0, 0, WIDTH, HEIGHT,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glfwSwapBuffers(win);
    glfwPollEvents();
  }

  free(parr.pixels);
  glfwTerminate();

  return 0;
}
