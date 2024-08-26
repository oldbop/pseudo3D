#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#ifdef _WIN32
#include <windows.h>

#define SLEEP_MS(ms) Sleep(ms)

#else
#include <sys/select.h>

#define SLEEP_MS(ms)                                      \
do {                                                      \
  struct timeval delay = { 0, ms * 1.0E+3 };              \
                                                          \
  if (delay.tv_usec > 0 && delay.tv_usec < 1.0E+6)        \
    select(0, NULL, NULL, NULL, &delay);                  \
} while(0)

#endif

#ifdef SYS_GL_HEADERS
#define GL_GLEXT_PROTOTYPES

#include <GL/gl.h>
#include <GL/glext.h>

#define LOAD_GL()

#else
#include <glad/gl.h>

#define LOAD_GL() gladLoadGL(glfwGetProcAddress)

#endif

#define COLOR(r, g, b, a) (r << 24) | (g << 16) | (b << 8)  | (a << 0)

#define TITLE   "pseudo3D"
#define MAP_W   8
#define MAP_H   8
#define SCR_W   960
#define SCR_H   720
#define DSCALE  4
#define FPS_CAP 2000
#define PI      3.141592653589793f

typedef float vec2f[2];
typedef float vec3f[3];
typedef int32_t vec2i[2];
typedef int32_t vec3i[3];

typedef uint8_t Map[MAP_W * MAP_H];

typedef struct {
  int32_t width, height, cx, cy, npixels;
  uint32_t *pixels, ftex, fbo;
  double dlastf, dlastt, minspf;
  char title[32];
  GLFWwindow *win;
} Renderer;

typedef struct {
  vec2f pos, dir;
  double time;
  Map map;
} GameState;

Renderer rdr = {
  .width   = SCR_W / DSCALE,
  .height  = SCR_H / DSCALE,
  .cx      = SCR_W / (DSCALE * 2),
  .cy      = SCR_H / (DSCALE * 2),
  .npixels = (SCR_W / DSCALE) * (SCR_H / DSCALE),
  .minspf  = 1.0 / FPS_CAP
};

GameState sta = {
  .pos = { MAP_W / 2.0f, MAP_H / 2.0f }, .dir = { 0.0f, 1.0f },
  .map = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 0, 1, 0, 1,
    1, 0, 0, 0, 1, 1, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1
  }
};

static inline void clear_screen(uint32_t color) {
  for (int32_t i = 0; i < rdr.npixels; ++i)
    rdr.pixels[i] = color;
}

static inline void draw_vert_line(uint32_t color, int32_t x, float len) {

  int32_t top = (rdr.cy + (len * rdr.cy)) + 0.5f;
  int32_t bot = (rdr.cy - (len * rdr.cy)) + 0.5f;

  for (int32_t i = bot; i < top; ++i)
    rdr.pixels[i * rdr.width + x] = color;
}

static inline void process_input(void) {
  if (glfwGetKey(rdr.win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(rdr.win, 1);
}

static inline void update_game(void) {

}

static inline void render(void) {

  glTexSubImage2D(GL_TEXTURE_2D,
                  0, 0, 0,
                  rdr.width, rdr.height,
                  GL_RGBA,
                  GL_UNSIGNED_INT_8_8_8_8,
                  rdr.pixels);

  glBlitFramebuffer(0, 0, rdr.width, rdr.height,
                    0, 0, SCR_W, SCR_H,
                    GL_COLOR_BUFFER_BIT,
                    GL_NEAREST);

  glfwSwapBuffers(rdr.win);
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

  rdr.win = glfwCreateWindow(SCR_W, SCR_H, TITLE, NULL, NULL);

  if (!rdr.win) {
    printf("GLFW: failed to create window\n");
    return -1;
  }

  rdr.pixels = malloc(rdr.npixels * sizeof(uint32_t));

  if (!rdr.pixels) {
    printf("Pixel array allocation failed\n");
    return -1;
  }

  glfwMakeContextCurrent(rdr.win);
  glfwSwapInterval(0);

  LOAD_GL();

  glViewport(0, 0, SCR_W, SCR_H);

  glGenTextures(1, &rdr.ftex);
  glBindTexture(GL_TEXTURE_2D, rdr.ftex);

  // Consider using GL_BGRA and GL_UNSIGNED_INT_8_8_8_8_REV
  glTexImage2D(GL_TEXTURE_2D,
               0, GL_RGBA,
               rdr.width, rdr.height, 0,
               GL_RGBA,
               GL_UNSIGNED_INT_8_8_8_8,
               NULL);

  glGenFramebuffers(1, &rdr.fbo);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, rdr.fbo);

  glFramebufferTexture2D(GL_READ_FRAMEBUFFER,
                         GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D,
                         rdr.ftex, 0);

  double lastf = 0.0, lastt = 0.0;

  while (!glfwWindowShouldClose(rdr.win)) {

    sta.time = glfwGetTime();

    rdr.dlastf = sta.time - lastf;
    rdr.dlastt = sta.time - lastt;

    process_input();

    if (rdr.dlastt >= 0.5) {
      snprintf(rdr.title, 32, "%s [FPS: %.2f]", TITLE, 1.0 / rdr.dlastf);
      glfwSetWindowTitle(rdr.win, rdr.title);
      lastt = sta.time;
    }

    clear_screen(COLOR(0x1a, 0x1a, 0x1a, 0xff));

    for (int32_t i = 0; i < rdr.width; ++i)
      draw_vert_line(COLOR(0xff, 0xff, 0xff, 0xff), i, 0.5f);

    render();
    lastf = sta.time;

    SLEEP_MS((sta.time + rdr.minspf - glfwGetTime()) * 1.0E+3);

    glfwPollEvents();
  }

  free(rdr.pixels);
  glfwTerminate();

  return 0;
}
