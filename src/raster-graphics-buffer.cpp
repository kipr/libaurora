#include <new>
#include <cstdlib>

#include <png.h>

#include "raster-graphics-buffer.hpp"

using namespace aurora;
using namespace std;

RasterGraphicsBuffer::RasterGraphicsBuffer(uint32_t width, uint32_t height)
  : _width(width)
  , _height(height)
  , _pixel_array(width * height, RGBPixel(0, 0, 0))
{

}

const RGBPixel &RasterGraphicsBuffer::pixel(uint32_t col, uint32_t row) const
{
  return _pixel_array.at(col + row * _width);
}

void RasterGraphicsBuffer::set_pixel(uint32_t col, uint32_t row, const RGBPixel &pixel)
{
  _pixel_array.at(col + row*_width) = pixel;
}

void RasterGraphicsBuffer::fill(const RGBPixel &color)
{
  _pixel_array.assign(_pixel_array.size(), color);
}

void RasterGraphicsBuffer::to_png(vector<uint8_t> &png_buffer) const
{
  const int bit_depth = 8;

  struct PngPtrRAIIWrapper
  {
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_bytep row = NULL;

    ~PngPtrRAIIWrapper()
    {
      if(info_ptr != NULL)
      {
        png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
      }

      if(png_ptr != NULL)
      {
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
      }

      if(row != NULL)
      {
        free(row);
      }
    }
  } rw;

  // create the PNG write struct
  rw.png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(rw.png_ptr == NULL)
  {
    throw bad_alloc();
  }

  // create the PNG info struct
  rw.info_ptr = png_create_info_struct(rw.png_ptr);
  if(rw.png_ptr == NULL)
  {
    throw bad_alloc();
  }

  // set the header
  png_set_IHDR(rw.png_ptr
    , rw.info_ptr
    , this->_width
    , this->_height
    , bit_depth
    , PNG_COLOR_TYPE_RGB
    , PNG_INTERLACE_NONE
    , PNG_COMPRESSION_TYPE_BASE
    , PNG_FILTER_TYPE_BASE
    );

  // set rows
  vector<uint8_t*> rows;
  for(uint32_t y = 0; y < this->_height; y++)
  {
    rows.push_back((uint8_t*)&this->_pixel_array[y * this->_width]);
  }
  png_set_rows(rw.png_ptr, rw.info_ptr, &rows[0]);

  // set write function
  png_set_write_fn(rw.png_ptr
    , (png_voidp)&png_buffer
    , [](png_structp png_ptr, png_bytep data, png_size_t length)
      {
        vector<uint8_t>* png_buffer = (vector<uint8_t>*) png_get_io_ptr(png_ptr);
        png_buffer->insert(png_buffer->end(), data, data + length);
      }
    , NULL);

  // write data
  if(setjmp(png_jmpbuf(rw.png_ptr)))
  {
    throw runtime_error("png_write_png");
  }

  png_write_png(rw.png_ptr, rw.info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
}