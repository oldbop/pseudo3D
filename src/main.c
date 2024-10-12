#include <math.h>
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

#define MIN(v, min) (v < min) ? min : v
#define MAX(v, max) (v > max) ? max : v
#define CLAMP(v, min, max) (v < min) ? min : ((v > max) ? max : v)
#define SIGN(v) (v > 0) ? 1 : ((v < 0) ? -1 : 0)

#define PI      3.141592653589793f

#define TITLE   "pseudo3D"
#define MAP_W   8
#define MAP_H   8
#define SCR_W   960
#define SCR_H   720
#define DSCALE  2.0f
#define FPS_CAP 2000
#define FOV     (PI / 3.0f)

#define DWIDTH  (SCR_W / DSCALE)
#define DHEIGHT (SCR_H / DSCALE)
#define CX      (DWIDTH / 2.0f)
#define CY      (DHEIGHT / 2.0f)
#define NPIXELS (DWIDTH * DHEIGHT)
#define MINSPF  (1.0 / FPS_CAP)

typedef float vec2f[2];
typedef int32_t vec2i[2];

typedef struct {
  vec2i pos, ustep;
  vec2f dir, mag, estep, offset;
  uint8_t hit, nsew;
} Ray;

typedef struct {
  uint32_t *pixels, ftex, fbo;
  double dlastf, dlastt;
  char title[32];
  GLFWwindow *win;
} Renderer;

typedef struct {
  vec2f pos, dir, proj;
  uint8_t map[MAP_W * MAP_H];
  double time;
} GameState;

static Renderer rdr;

static GameState sta = {
  .pos = { MAP_W / 2.0f, MAP_H / 2.0f },
  .dir = { 0.0f, 1.0f },
  .map = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 1, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1
  }
};

static inline void process_input(void) {

  if (glfwGetKey(rdr.win, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(rdr.win, 1);

  if (glfwGetKey(rdr.win, GLFW_KEY_W) == GLFW_PRESS)
    sta.pos[1] += 0.01f;

  if (glfwGetKey(rdr.win, GLFW_KEY_S) == GLFW_PRESS)
    sta.pos[1] -= 0.01f;

  if (glfwGetKey(rdr.win, GLFW_KEY_D) == GLFW_PRESS)
    sta.pos[0] += 0.01f;

  if (glfwGetKey(rdr.win, GLFW_KEY_A) == GLFW_PRESS)
    sta.pos[0] -= 0.01f;

  sta.pos[0] = CLAMP(sta.pos[0], 1.1f, MAP_W - 1.1f);
  sta.pos[1] = CLAMP(sta.pos[1], 1.1f, MAP_H - 1.1f);
}

static inline void clear_screen(uint32_t color) {

  for (uint32_t i = 0; i < NPIXELS; ++i)
    rdr.pixels[i] = color;
}

static inline void draw_vert_line(uint32_t color, uint32_t x, float len) {

  len = CLAMP(len, 0.0f, 1.0f);

  uint32_t top = ((uint32_t) CY + (len * (uint32_t) CY)) + 0.5f;
  uint32_t bot = ((uint32_t) CY - (len * (uint32_t) CY)) + 0.5f;

  for (uint32_t i = bot; i < top; ++i)
    rdr.pixels[x + i * (uint32_t) DWIDTH] = color;
}

static inline void render(void) {

  glTexSubImage2D(GL_TEXTURE_2D,
                  0, 0, 0,
                  DWIDTH, DHEIGHT,
                  GL_RGBA,
                  GL_UNSIGNED_INT_8_8_8_8,
                  rdr.pixels);

  glBlitFramebuffer(0, 0, DWIDTH, DHEIGHT,
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

  rdr.pixels = malloc(NPIXELS * sizeof(uint32_t));

  if (!rdr.pixels) {
    printf("Pixel array allocation failed\n");
    return -1;
  }

  sta.proj[0] = tan(FOV / 2.0f);
  sta.proj[1] = 0.0f;

  glfwMakeContextCurrent(rdr.win);
  glfwSwapInterval(0);

  LOAD_GL();

  glViewport(0, 0, SCR_W, SCR_H);

  glGenTextures(1, &rdr.ftex);
  glBindTexture(GL_TEXTURE_2D, rdr.ftex);

  // Consider using GL_BGRA and GL_UNSIGNED_INT_8_8_8_8_REV
  glTexImage2D(GL_TEXTURE_2D,
               0, GL_RGBA,
               DWIDTH, DHEIGHT, 0,
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

    for (uint32_t i = 0; i < DWIDTH; ++i) {

      float ndc = (2.0f * i / DWIDTH) - 1.0f;

      Ray ray = {
        .pos = { sta.pos[0], sta.pos[1] },
        .dir = {
          sta.dir[0] + (sta.proj[0] * ndc),
          sta.dir[1] + (sta.proj[1] * ndc)
        }
      };

      ray.ustep[0] = SIGN(ray.dir[0]);
      ray.ustep[1] = SIGN(ray.dir[1]);

      // Taking |ray.dir| = 1
      ray.estep[0] = (ray.dir[0] == 0) ? 1.0E+20 : fabs(1.0f / ray.dir[0]);
      ray.estep[1] = (ray.dir[1] == 0) ? 1.0E+20 : fabs(1.0f / ray.dir[1]);

      ray.offset[0] = (ray.dir[0] >= 0) ? (ray.pos[0] + 1.0f - sta.pos[0]) : (sta.pos[0] - ray.pos[0]);
      ray.offset[1] = (ray.dir[1] >= 0) ? (ray.pos[1] + 1.0f - sta.pos[1]) : (sta.pos[1] - ray.pos[1]);

      ray.mag[0] = ray.offset[0] * ray.estep[0];
      ray.mag[1] = ray.offset[1] * ray.estep[1];

      while (!ray.hit) {
        if (ray.mag[0] < ray.mag[1]) {  
          ray.mag[0] += ray.estep[0];
          ray.pos[0] += ray.ustep[0];
          ray.nsew = 0;
        } else {
          ray.mag[1] += ray.estep[1];
          ray.pos[1] += ray.ustep[1];
          ray.nsew = 1;
        }
        ray.hit = sta.map[ray.pos[1] * MAP_W + ray.pos[0]];
      }

      float walldist = (ray.nsew == 1) ? (ray.mag[1] - ray.estep[1]) : (ray.mag[0] - ray.estep[0]);

      if (ray.nsew) {
        draw_vert_line(COLOR(0xff, 0xff, 0x00, 0xff), i, 1.0f / walldist);
      } else {
        draw_vert_line(COLOR(0xff, 0x00, 0xff, 0xff), i, 1.0f / walldist);
      }
    }

    render();
    lastf = sta.time;

    SLEEP_MS((sta.time + MINSPF - glfwGetTime()) * 1.0E+3);

    glfwPollEvents();
  }

  free(rdr.pixels);
  glfwTerminate();

  return 0;
}
