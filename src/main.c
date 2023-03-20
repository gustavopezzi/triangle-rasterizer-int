#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>
#include "display.h"
#include "vec2.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

bool is_running = false;

vec2_t vertices[4] = {
  { .x = 40, .y = 40 },
  { .x = 80, .y = 40 },
  { .x = 40, .y = 80 },
  { .x = 90, .y = 90 }
};

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} color_t;

color_t colors[3] = {
  { .r = 0xFF, .g = 0x00, .b = 0x00 },
  { .r = 0x00, .g = 0xFF, .b = 0x00 },
  { .r = 0x00, .g = 0x00, .b = 0xFF }
};

void process_input(void) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        is_running = false;
        break;
      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_ESCAPE)
          is_running = false;
        break;
    }
  }
}

bool is_top_left(vec2_t* start, vec2_t* end) {
  vec2_t edge = { end->x - start->x, end->y - start->y };
  bool is_top_edge = edge.y == 0 && edge.x > 0;
  bool is_left_edge = edge.y < 0;
  return is_left_edge || is_top_edge;
}

int edge_cross(vec2_t* a, vec2_t* b, vec2_t* p) {
  vec2_t ab = { b->x - a->x, b->y - a->y };
  vec2_t ap = { p->x - a->x, p->y - a->y };
  return ab.x * ap.y - ab.y * ap.x;
}

void triangle_fill(vec2_t v0, vec2_t v1, vec2_t v2, uint32_t color) {
  // Finds the bounding box with all candidate pixels
  int x_min = MIN(MIN(v0.x, v1.x), v2.x);
  int y_min = MIN(MIN(v0.y, v1.y), v2.y);
  int x_max = MAX(MAX(v0.x, v1.x), v2.x);
  int y_max = MAX(MAX(v0.y, v1.y), v2.y);

  // Compute the constant delta_s that will be used for the horizontal and vertical steps
  int delta_w0_col = (v1.y - v2.y);
  int delta_w1_col = (v2.y - v0.y);
  int delta_w2_col = (v0.y - v1.y);
  int delta_w0_row = (v2.x - v1.x);
  int delta_w1_row = (v0.x - v2.x);
  int delta_w2_row = (v1.x - v0.x);

  // Rasterization fill convention (top-left rule)
  int bias0 = is_top_left(&v1, &v2) ? -1 : 0;
  int bias1 = is_top_left(&v2, &v0) ? -1 : 0;
  int bias2 = is_top_left(&v0, &v1) ? -1 : 0;

  // Compute the edge functions for the fist (top-left) point
  vec2_t p0 = { x_min, y_min };
  int w0_row = edge_cross(&v1, &v2, &p0) + bias0;
  int w1_row = edge_cross(&v2, &v0, &p0) + bias1;
  int w2_row = edge_cross(&v0, &v1, &p0) + bias2;

  // Loop all candidate pixels inside the bounding box
  for (int y = y_min; y <= y_max; y++) {
    int w0 = w0_row;
    int w1 = w1_row;
    int w2 = w2_row;
    for (int x = x_min; x <= x_max; x++) {
      bool is_inside = w0 >= 0 && w1 >= 0 && w2 >= 0;
      if (is_inside) {
        draw_pixel(x, y, color);
      }
      w0 += delta_w0_col;
      w1 += delta_w1_col;
      w2 += delta_w2_col;
    }
    w0_row += delta_w0_row;
    w1_row += delta_w1_row;
    w2_row += delta_w2_row;
  }
}

void render(void) {
  clear_framebuffer(0xFF000000);

  float angle = SDL_GetTicks() / 1000.0f * 0.2;

  vec2_t center = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };

  vec2_t v0 = vec2_rotate(vertices[0], center, angle);
  vec2_t v1 = vec2_rotate(vertices[1], center, angle);
  vec2_t v2 = vec2_rotate(vertices[2], center, angle);
  vec2_t v3 = vec2_rotate(vertices[3], center, angle);

  triangle_fill(v0, v1, v2, 0xFF00FF00);
  triangle_fill(v3, v2, v1, 0xFFA74DE3);

  render_framebuffer();
}

int main(void) {
  is_running = create_window();

  while (is_running) {
    fix_framerate();
    process_input();
    render();
  }

  destroy_window();

  return EXIT_SUCCESS;
}