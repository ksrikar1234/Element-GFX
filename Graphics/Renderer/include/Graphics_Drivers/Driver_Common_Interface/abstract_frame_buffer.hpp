#ifndef _GP_ABSTRACT_FRAME_BUFFER_CLASS_H_
#define _GP_ABSTRACT_FRAME_BUFFER_CLASS_H_

#include <vector>
#include <cstdint>
#include "gp_gui_debug.h"
#include "gp_gui_typedefs.h"
#include <cstdint>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <numeric>
#include <set>

namespace gridpro_gui
{
  class Abstract_Framebuffer
  {
  public:
    Abstract_Framebuffer();
    virtual ~Abstract_Framebuffer() = default;

    enum class ScanMode
    {
      PER_PIXEL,
      LEFT_RIGHT,
      SPIRAL
    };

    ScanMode scan_mode;

    virtual void update_current_frame_buffer()
    {
      GP_ERROR("This function should be implemented in the derived class");
    }

    uint32_t color_id_at(const float current_mouse_x, const float current_mouse_y);
    std::vector<uint32_t> pick_matrix(const float current_mouse_x, const float current_mouse_y, const float pick_matrix_length, const float pick_matrix_width);
    uint32_t pixel_at(const float current_mouse_x, const float current_mouse_y);
    const std::vector<unsigned char> *data();
    float depth_at(const float current_mouse_x, const float current_mouse_y);
    float last_hit_depth();
    inline std::vector<uint32_t> scanline_polygon(const std::vector<float>& in_polygon);

  protected:
    std::vector<unsigned char> framebufferData;
    std::vector<float> DepthBufferData;
    uint32_t framebufferWidth;
    uint32_t framebufferHeight;
    uint32_t color_id;
    float last_hit_x, last_hit_y;
  };

  /// @brief Constructor
  inline Abstract_Framebuffer::Abstract_Framebuffer()
  {
    scan_mode = ScanMode::LEFT_RIGHT;
  }

  /// @brief Get the color id at the specified mouse coordinates
  inline uint32_t Abstract_Framebuffer::color_id_at(const float current_mouse_x, const float current_mouse_y)
  {
    //----------------------------------------------------------------------------------------------------------------
    // Here we are reading only one pixel , for using a pickmatrix iterate through every pixel in the pick matrix.
    // ----> current_mouse_x , viewport[3] - current_mouse_y is the origin [0,0].
    //-----------------------------------------------------------------------------------------------------------------
    //                Pixel Pick Matrix
    //                         x-----------+
    //                [-1, 1][0, 1][1, 1]  | y_max
    //                [-1, 0][0, 0][1, 0]  y
    //                [-1,-1][0,-1][1,-1]  | y_min
    //-----------------------------------------------------------------------------------------------------------------

    const float pick_matrix_length = 5;
    const float pick_matrix_width = 5;

    if (scan_mode == ScanMode::LEFT_RIGHT)
    {
      auto hits =  pick_matrix(current_mouse_x, current_mouse_y, pick_matrix_length, pick_matrix_width);
      return hits.size() > 0 ? hits[0] : 0;
    }

    else if (scan_mode == ScanMode::SPIRAL)
    {
      int x = static_cast<int>(current_mouse_x);
      int y = static_cast<int>(current_mouse_y);
      // Define the spiral parameters
      int dx = 1;
      int dy = 0;
      int segment_length = 1;
      int segment_passed = 0;

      for (int i = 0; i < pick_matrix_length; ++i)
      {
        for (int j = 0; j < pick_matrix_width; ++j)
        {
          // Ensure the current coordinates are within the bounds of your matrix
          if (x >= 0 && x < framebufferWidth && y >= 0 && y < framebufferHeight)
          {
            uint32_t hit = pixel_at(x, y);
            if (hit != 0)
            {
              /*DEBUG_PRINT("Hit at", x, y, "Pixel ID =", hit);*/
              return hit;
            }
          }

          x += dx;
          y += dy;
          ++segment_passed;

          if (segment_passed == segment_length)
          {
            segment_passed = 0;

            // Change direction in a counter-clockwise spiral
            int temp = dx;
            dx = -dy;
            dy = temp;

            // Increase segment length every two turns
            if (dy == 0)
            {
              ++segment_length;
            }
          }
        }
      }
    }

    else
    {
      uint32_t hit = pixel_at(current_mouse_x, current_mouse_x);
      if (hit != 0)
        return hit;
    }
    return 0;
  }

  /// @brief Get the color id at the specified mouse coordinates with pick matrix
  inline std::vector<uint32_t> Abstract_Framebuffer::pick_matrix(const float current_mouse_x, const float current_mouse_y, const float pick_matrix_length = 4, const float pick_matrix_width = 4)
  {
    //----------------------------------------------------------------------------------------------------------------
    // Here we are reading only one pixel , for using a pickmatrix iterate through every pixel in the pick matrix.
    // ----> current_mouse_x , viewport[3] - current_mouse_y is the origin [0,0].
    //-----------------------------------------------------------------------------------------------------------------
    //                Pixel Pick Matrix
    //                         x-----------+
    //                [-1, 1][0, 1][1, 1]  | y_max
    //                [-1, 0][0, 0][1, 0]  y
    //                [-1,-1][0,-1][1,-1]  | y_min
    //-----------------------------------------------------------------------------------------------------------------

    std::vector<uint32_t> hits;

    for (float i = (current_mouse_x); i < (current_mouse_x + (pick_matrix_length / 2)); ++i)
      for (float j = (current_mouse_y); j < (current_mouse_y + (pick_matrix_width / 2)); ++j)
      {
        uint32_t hit = pixel_at(i, j);
        if (hit != 0)
        {
          GP_TRACE("Hit at", i, j, "Pixel ID = ", hit);
          hits.push_back(hit);
          last_hit_x = i;
          last_hit_y = j;
        }
      }

    for (float i = (current_mouse_x - (pick_matrix_length / 2)); i < (current_mouse_x); ++i)
      for (float j = (current_mouse_y - (pick_matrix_width / 2)); j < (current_mouse_y); ++j)
      {
        uint32_t hit = pixel_at(i, j);
        if (hit != 0)
        {
          GP_TRACE("Hit at", i, j, "Pixel ID = ", hit);
          hits.push_back(hit);
          last_hit_x = i;
          last_hit_y = j;
        }
      }

    return hits;
  }

  /// @brief Get the depth at the specified mouse coordinates
  inline float Abstract_Framebuffer::depth_at(const float current_mouse_x, const float current_mouse_y)
  {
    // Calculate the pixel coordinates in the framebuffer
    int pixelX = static_cast<int>(current_mouse_x);
    int pixelY = framebufferHeight - static_cast<int>(current_mouse_y) - 1;
    if (pixelX >= 0 && pixelX < framebufferWidth && pixelY >= 0 && pixelY < framebufferHeight)
    {
      // Calculate the index in the DepthBuffer data array
      int index = (pixelY * framebufferWidth + pixelX);
      return DepthBufferData[index];
    }
    return 0;
  }

  /// @brief Get the last hit depth
  inline float Abstract_Framebuffer::last_hit_depth()
  {
    return depth_at(last_hit_x, last_hit_y);
  }

  /// @brief Get the pixel at the specified mouse coordinates
  inline uint32_t Abstract_Framebuffer::pixel_at(const float current_mouse_x, const float current_mouse_y)
  {
    // Calculate the pixel coordinates in the framebuffer
    int pixelX = static_cast<int>(current_mouse_x);
    int pixelY = framebufferHeight - static_cast<int>(current_mouse_y) - 1;
    if (pixelX >= 0 && pixelX < framebufferWidth && pixelY >= 0 && pixelY < framebufferHeight)
    {
      // Calculate the index in the framebuffer data array
      int index = (pixelY * framebufferWidth + pixelX) * 4; // Assuming RGBA format

      // Get the pixel values at the specified coordinates
      GLubyte r = framebufferData[index];
      GLubyte g = framebufferData[index + 1];
      GLubyte b = framebufferData[index + 2];
      GLubyte a = framebufferData[index + 3];

      GLuint Selected_Primitive_ID = 0;
      Selected_Primitive_ID = ((Selected_Primitive_ID + b) << 8);
      Selected_Primitive_ID = ((Selected_Primitive_ID + g) << 8);
      Selected_Primitive_ID = (Selected_Primitive_ID + r);

      // Print or use the pixel values as needed
      // std::cout << "Pixel at (" << pixelX << ", " << pixelY << "): ";
      // std::cout << "Selected Corner ID using frame buffer = " << Selected_Primitive_ID  << "\n";
      // std::cout << "R: " << static_cast<int>(r) << " G: " << static_cast<int>(g) << " B: " << static_cast<int>(b) << " A: " << static_cast<int>(a) << std::endl;
      color_id = Selected_Primitive_ID;
      return Selected_Primitive_ID;
    }

    return 0;
  }

  /// @brief Get the framebuffer data
  inline const std::vector<unsigned char> *Abstract_Framebuffer::data()
  {
    return &framebufferData;
  }
inline std::vector<uint32_t> Abstract_Framebuffer::scanline_polygon(const std::vector<float>& in_polygon) {
    // Inner struct for representing integer-aligned points
    struct Point {
        int x, y;
    };
    

    // Convert input polygon (float coordinates) to integer points
    std::vector<Point> polygon;
    for (size_t i = 0; i < in_polygon.size(); i += 2) {
        polygon.push_back({static_cast<int>(std::round(in_polygon[i])),
                           static_cast<int>(std::round(in_polygon[i + 1]))});
    }


    // Ensure the polygon is closed
    // if (polygon.empty() || polygon.front().x != polygon.back().x || polygon.front().y != polygon.back().y) {
    //     throw std::invalid_argument("Polygon is not closed.");
    // }

     // Simply the ensure the polygon is closed
     if(polygon.empty())
     {
         GP_ERROR("Polygon is empty");
         return {};
     }
       
     // Just ensure the first and last points are near to each other with delta of 5 pixels
    //  if(std::abs(polygon.front().x - polygon.back().x) > 5 || std::abs(polygon.front().y - polygon.back().y) > 5)
    //  {
    //       GP_ERROR("Polygon is not closed");
    //       GP_PRINT("Polygon first = ", polygon.front().x, polygon.front().y, "Polygon last = ", polygon.back().x, polygon.back().y);
    //       // polygon.pop_back();
    //       // polygon.push_back(polygon.front());
    //  }
     
    GP_COLOR_PRINT(GP_COLOR::GREEN, "Starting Raster Scanline Algorithm");

    std::set<uint32_t> result;

    // Find the bounding box of the polygon
    int min_y = std::numeric_limits<int>::max();
    int max_y = std::numeric_limits<int>::min();
    for (const auto& p : polygon) {
        min_y = std::min(min_y, p.y);
        max_y = std::max(max_y, p.y);
    }

    // Iterate through each scanline
    for (int y = min_y; y <= max_y; ++y) {
        // Find intersections of the scanline with the polygon edges
        std::vector<int> intersections;
        for (size_t i = 0; i < polygon.size() - 1; ++i) {
            const Point& p1 = polygon[i];
            const Point& p2 = polygon[i + 1];

            // Check if the scanline intersects the edge
            if ((p1.y <= y && p2.y > y) || (p2.y <= y && p1.y > y)) {
                // Compute the x-coordinate of the intersection
                float slope = static_cast<float>(p2.x - p1.x) / (p2.y - p1.y);
                int x_intersect = static_cast<int>(std::round(p1.x + (y - p1.y) * slope));
                intersections.push_back(x_intersect);
            }
        }

        // Sort the intersections to determine pixel spans
        std::sort(intersections.begin(), intersections.end());

        // Fill pixels between pairs of intersections
        for (size_t i = 0; i < intersections.size(); i += 2) {
            if (i + 1 < intersections.size()) {
                int start_x = intersections[i];
                int end_x = intersections[i + 1];
                for (int x = start_x; x <= end_x; ++x) {
                    // Capture the pixel at (x, y) from the framebuffer
                    // Fetch pixel value
                    uint32_t pixel_value = pixel_at(static_cast<float>(x), static_cast<float>(y));
                    if(pixel_value != 0)
                    {
                      result.insert(pixel_value);
                    }
                }
            }
        }
    }
    
    std::vector<uint32_t> result_vec(result.begin(), result.end());


    return result_vec;
}
}

#endif