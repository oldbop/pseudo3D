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

  const int32_t dwidth = WIDTH / DSCALE;
  const int32_t dheight = HEIGHT / DSCALE;
  const int32_t npixels = dwidth * dheight;

  printf("Screen width: %dpx\tDownscaled width: %dpx\n"
         "Screen height: %dpx\tDownscaled height: %dpx\n",
         WIDTH, dwidth, HEIGHT, dheight);

  uint32_t *colorbuf = malloc(npixels * sizeof(uint32_t));

  glfwMakeContextCurrent(win);
  glfwSwapInterval(1);

  LOAD_GL();

  glViewport(0, 0, WIDTH, HEIGHT);

  uint32_t ftex;
  glGenTextures(1, &ftex);
  glBindTexture(GL_TEXTURE_2D, ftex);

  // Consider using GL_BGRA and GL_UNSIGNED_INT_8_8_8_8_REV
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, dwidth, dheight, 0, GL_RGBA,
               GL_UNSIGNED_INT_8_8_8_8, NULL);

  uint32_t fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);

  glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, ftex, 0);

  while (!glfwWindowShouldClose(win)) {

    /*
    double time = glfwGetTime();

    uint8_t r = (sin(time) + 1) * 127;
    uint8_t g = (cos(time) + 1) * 127;
    uint8_t b = (cos(time + PI) + 1) * 127;
    */

    uint32_t grey = COLOR(0x1a, 0x1a, 0x1a, 0xff);

    // Use vector intrinsics here...
    for (uint32_t i = 0; i < npixels; ++i) {
      colorbuf[i] = grey;
    }

    uint32_t white = COLOR(0xff, 0xff, 0xff, 0xff);

    for (uint32_t i = 20000; i < 20050; ++i) {
      colorbuf[i] = white;
    }

    // Is there another way of doing this?
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, dwidth, dheight, GL_RGBA,
                    GL_UNSIGNED_INT_8_8_8_8, colorbuf);

    // Is this optimal?
    glBlitFramebuffer(0, 0, dwidth, dheight, 0, 0, WIDTH, HEIGHT,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glfwSwapBuffers(win);
    glfwPollEvents();
  }

  free(colorbuf);
  glfwTerminate();

  return 0;
}
