#pragma once
#ifndef _GRIDPRO_GL_PIXEL_UTILS_CLASS_HPP_
#define _GRIDPRO_GL_PIXEL_UTILS_CLASS_HPP_

#include <cmath>
#include <cstdint>

namespace GridPro_GFX
{

  class PixelData
  {
  public:
    union
    {
      uint32_t color_id;
      uint8_t rgba[4];
      struct
      {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
      };
    };

    PixelData(const float red, const float green, const float blue, const float alpha = 1.0f)
    {
      rgba[0] = uint8_t(red * 255);
      rgba[1] = uint8_t(green * 255);
      rgba[2] = uint8_t(blue * 255);
      rgba[3] = uint8_t(alpha * 255);
    }

    PixelData(const uint8_t red, const uint8_t green, const uint8_t blue, const uint8_t alpha = 255)
    {
      rgba[0] = (red);
      rgba[1] = (green);
      rgba[2] = (blue);
      rgba[3] = (alpha);
    }

    PixelData(const uint32_t color_id) : color_id(color_id) {}

    uint8_t &operator[](const int32_t& index)
    {
        return rgba[index];
    }

    float r_float() { return float(rgba[0]) / 255.0f; }
    float g_float() { return float(rgba[1]) / 255.0f; }
    float b_float() { return float(rgba[2]) / 255.0f; }
    float a_float() { return float(rgba[3]) / 255.0f; }

    uint8_t r_ubyte() { return rgba[0]; }
    uint8_t g_ubyte() { return rgba[1]; }
    uint8_t b_ubyte() { return rgba[2]; }
    uint8_t a_ubyte() { return rgba[3]; }

    uint32_t get_color_id() { return color_id; }
    void set_color_id(const uint32_t &input_color_id) { color_id = input_color_id; }
  };
} // namespace HELIUM_API

#endif