#ifndef _PNG_FRAME_BUFFER_HPP_
#define _PNG_FRAME_BUFFER_HPP_

#include <stdint.h>
#include <vector>

namespace aurora
{

  struct RGBPixel
  {
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    RGBPixel(uint8_t red, uint8_t green, uint8_t blue)
      : red(red), green(green), blue(blue) {}
  };

  class RasterGraphicsBuffer
  {
  public:
    RasterGraphicsBuffer(uint32_t width, uint32_t height);

    const RGBPixel &pixel(uint32_t col, uint32_t row) const;
    void set_pixel(uint32_t col, uint32_t row, const RGBPixel &pixel);
    void fill(const RGBPixel &color);

    void to_png(std::vector<uint8_t> &png_buffer) const;

  private:
    uint32_t _width;
    uint32_t _height;
    std::vector<RGBPixel> _pixel_array;
  };

}

#endif
