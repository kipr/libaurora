#include <algorithm>
#include <cstring>
#include <cmath>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "kipr/graphics.h"

#include "daylite-node.hpp"
#include "raster-graphics-buffer.hpp"

using namespace std;
using namespace aurora;

namespace
{
  struct
  {
    struct
    {
      int32_t x;
      int32_t y;
    } pos;

    struct
    {
      bool left;
      bool middle;
      bool right;
    } button_down;
  } g_mouse_state;

  struct
  {
    unordered_set<KeyCode> pressed;
  } g_key_state;

  unique_ptr<RasterGraphicsBuffer> g_graphics_buffer;
  uint32_t g_width;
  uint32_t g_height;
}

int graphics_open(int width, int height)
{
  if(g_graphics_buffer)
  {
    cerr << "Graphics already open" << endl;
    return false;
  }

  if(!daylite_node::ref().start())
  {
    return false;
  }

  daylite_node::ref().set_key_events_callback([](unordered_set<KeyCode> pressed_keys)
  {
    pressed_keys.swap(g_key_state.pressed);
  });

  daylite_node::ref().set_mouse_event_callback([](const int32_t *pos_x
    , const int32_t *pos_y
    , const bool *left_down
    , const bool *middle_down
    , const bool *right_down)
  {
    if(pos_x) g_mouse_state.pos.x = *pos_x;
    if(pos_y) g_mouse_state.pos.y = *pos_y;

    if(left_down) g_mouse_state.button_down.left = *left_down;
    if(middle_down) g_mouse_state.button_down.middle = *middle_down;
    if(right_down) g_mouse_state.button_down.right = *right_down;
  });

  try
  {
    g_graphics_buffer = make_unique<RasterGraphicsBuffer>(width, height);
  }
  catch(...)
  {
    return false;
  }

  g_width = width;
  g_height = height;

  // workaround to avoid missing graphics updates
  this_thread::sleep_for(chrono::milliseconds(600));
  daylite_node::ref().spin_once();
  this_thread::sleep_for(chrono::milliseconds(200));
  daylite_node::ref().spin_once();
  this_thread::sleep_for(chrono::milliseconds(200));

  return true;
}

void graphics_close()
{
  // workaround to avoid missing graphics updates
  this_thread::sleep_for(chrono::milliseconds(500));

  daylite_node::ref().end();

  g_graphics_buffer.release();
}

void graphics_update()
{
  vector<uint8_t> buffer;
  g_graphics_buffer->to_png(buffer);

  daylite_node::ref().publish_frame("PNG", g_width, g_height, buffer);

  daylite_node::ref().spin_once();
}

void graphics_clear()
{
  graphics_fill(0, 0, 0);
}

void graphics_blit(const unsigned char *data, int x, int y, int width, int height)
{
  graphics_blit_enc(data, RGB, x, y, width, height);
}

void graphics_blit_region(const unsigned char *data, int sx, int sy, int ex, int ey, int width, int height, int dx, int dy)
{
  graphics_blit_region_enc(data, RGB, sx, sy, ex, ey, width, height, dx, dy);
}

void graphics_blit_enc(const unsigned char *data, Encoding enc, int x, int y, int width, int height)
{
  graphics_blit_region_enc(data, enc, 0, 0, width - 1, height - 1, width, height, x, y);
}

void graphics_blit_section(const unsigned char *data, Encoding enc, const int index, const int dindex, const int length)
{
  //int actualLength = length + dindex < width * height ? length : width * height - dindex;
  //
  //if (actualLength > width * height) actualLength = width * height;
  //if(actualLength <= 0) return;
  //
  //for(int i = 0; i < actualLength; ++i) {
  //	const unsigned int offset = (i + index) * 3;
  //	g_graphics.pixels[dindex + i] = fromTrueColor(enc, data[offset],
  //		data[offset + 1], data[offset + 2]);
  //}

  cerr << "this function is not implemented yet" << endl;
}

void graphics_blit_region_enc(const unsigned char *data, Encoding enc, int sx, int sy, int ex, int ey, int width, int height, int dx, int dy)
{
  if(sx >= ex || sy >= ey) return;
  if(dx >= width || dy >= height) return;
  ey = ey > height - 1 ? height - 1 : ey;

  const int hsize = ey - sy + 1;
  for(int i = 0; i < hsize; ++i) {
    int cols = ex - sx + 1;
    if(cols + dx > width) cols -= dx;
    const int index = (sy + i) * width + sx;
    const int dindex = (dy + i) * width + dx;
    graphics_blit_section(data, enc,
      index < 0 ? 0 : index,
      dindex < 0 ? 0 : dindex,
      cols);
  }
}

void graphics_fill(int r, int g, int b)
{
  g_graphics_buffer->fill(RGBPixel(r, g, b));
}

void graphics_pixel(int x, int y, int r, int g, int b)
{
  if(x < 0 || x >= g_width) return;
  if(y < 0 || y >= g_height) return;

  g_graphics_buffer->set_pixel(x, y, RGBPixel(r, g, b));
}

void graphics_line(int x1, int y1, int x2, int y2, int r, int g, int b)
{
  int steep = abs(y2 - y1) > abs(x2 - x1) ? 1 : 0;

  if(steep) {
    swap(x1, y1);
    swap(x2, y2);
  }

  if(x1 > x2) {
    swap(x1, x2);
    swap(y1, y2);
  }

  int deltax = x2 - x1;
  int deltay = abs(y2 - y1);
  int error = -(deltax + 1) >> 1;
  int y = y1;

  int ystep = y1 < y2 ? 1 : -1;

  for(int x = x1; x <= x2; ++x) {
    if(steep) graphics_pixel(y, x, r, g, b);
    else graphics_pixel(x, y, r, g, b);
    error += deltay;
    if(error >= 0) {
      y += ystep;
      error -= deltax;
    }
  }
}

void graphics_circle(int cx, int cy, int radius, int r, int g, int b)
{
  if(radius <= 0) return;

  int f = 1 - radius;
  int ddF_x = 0;
  int ddF_y = -2 * radius;

  graphics_pixel(cx, cy + radius, r, g, b);
  graphics_pixel(cx, cy - radius, r, g, b);
  graphics_pixel(cx + radius, cy, r, g, b);
  graphics_pixel(cx - radius, cy, r, g, b);

  int x = 0;
  int y = radius;
  while(x < y) {
    if(f >= 0) {
      --y;
      ddF_y += 2;
      f += ddF_y;
    }
    ++x;
    ddF_x += 2;
    f += ddF_x + 1;
    graphics_pixel(cx + x, cy + y, r, g, b);
    graphics_pixel(cx - x, cy + y, r, g, b);
    graphics_pixel(cx + x, cy - y, r, g, b);
    graphics_pixel(cx - x, cy - y, r, g, b);
    graphics_pixel(cx + y, cy + x, r, g, b);
    graphics_pixel(cx - y, cy + x, r, g, b);
    graphics_pixel(cx + y, cy - x, r, g, b);
    graphics_pixel(cx - y, cy - x, r, g, b);
  }
}

void graphics_circle_fill(int cx, int cy, int radius, int r, int g, int b)
{
  if(radius <= 0) return;

  const long c_squared = (long)radius * (long)radius;

  for(int i = cx - radius; i < cx + radius; ++i) {

    for(int j = cy - radius; j < cy + radius; ++j) {
      long dx = cx - i;
      dx *= dx;

      long dy = cy - j;
      dy *= dy;

      if(dx + dy <= c_squared) {
        graphics_pixel(i, j, r, g, b);
      }
    }
  }
}

void graphics_rectangle(int x1, int y1, int x2, int y2, int r, int g, int b)
{
  graphics_line(x1, y1, x1, y2, r, g, b);
  graphics_line(x1, y1, x2, y1, r, g, b);
  graphics_line(x2, y2, x2, y1, r, g, b);
  graphics_line(x2, y2, x1, y2, r, g, b);
}

void graphics_rectangle_fill(int x1, int y1, int x2, int y2, int r, int g, int b)
{
  if(x1 > x2) swap(x1, x2);
  if(y1 > y2) swap(y1, y2);

  for(int i = x1; i <= x2; ++i) {
    for(int j = y1; j <= y2; ++j) {
      graphics_pixel(i, j, r, g, b);
    }
  }
}

void graphics_triangle(int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b)
{
  graphics_line(x1, y1, x2, y2, r, g, b);
  graphics_line(x2, y2, x3, y3, r, g, b);
  graphics_line(x3, y3, x1, y1, r, g, b);
}

void graphics_triangle_fill(int x1, int y1, int x2, int y2, int x3, int y3, int r, int g, int b)
{
  const double l = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
  if(l == 0) return;

  const double dx = (x2 - x1) / l;
  const double dy = (y2 - y1) / l;

  for(int i = 0; i <= l; ++i)
  {
    graphics_line(x3, y3, x1 + ceil(i * dx), y1 + i * dy, r, g, b);
    graphics_line(x3, y3, x1 + i * dx, y1 + ceil(i * dy), r, g, b);
    graphics_line(x3, y3, x1 + i * dx, y1 + i * dy, r, g, b);
    graphics_line(x3, y3, x1 + ceil(i * dx), y1 + ceil(i * dy), r, g, b);
  }
}


int get_key_state(enum KeyCode key)
{
  return (g_key_state.pressed.find(key) != g_key_state.pressed.end()) ? 1 : 0;
}

void get_mouse_position(int *x, int *y)
{
  *x = g_mouse_state.pos.x;
  *y = g_mouse_state.pos.y;
}

int get_mouse_middle_button()
{
  return g_mouse_state.button_down.middle;
}

int get_mouse_left_button()
{
  return g_mouse_state.button_down.left;
}

int get_mouse_right_button()
{
  return g_mouse_state.button_down.right;
}
