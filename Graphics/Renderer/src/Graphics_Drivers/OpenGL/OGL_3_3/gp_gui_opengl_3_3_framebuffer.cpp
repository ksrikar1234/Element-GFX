
#include "gp_gui_opengl_3_3_framebuffer.h"
#include "graphics_api.hpp"
#include "gp_gui_debug.h"

namespace gridpro_gui
{
  namespace OpenGL_3_3
  {
    /// @brief Constructor
    framebuffer::framebuffer() : Abstract_Framebuffer()
    {
      scan_mode = ScanMode::LEFT_RIGHT;
    }

    /// @brief Destructor
    framebuffer::~framebuffer() {}

    /// @brief Update the current framebuffer data
    void framebuffer::update_current_frame_buffer()
    {
      GLint viewport[4];
      RendererAPI<QGL_3_3>()->glGetIntegerv(GL_VIEWPORT, viewport);
    
    // Required for QGLWidget
    //RendererAPI<QGL_3_3>()->glReadBuffer(GL_BACK);

      framebufferWidth  = viewport[2];
      framebufferHeight = viewport[3];
      framebufferData.resize(framebufferWidth * framebufferHeight * 4);
      DepthBufferData.resize(framebufferData.size() / 4);
      RendererAPI<QGL_3_3>()->glReadPixels(0, 0, framebufferWidth, framebufferHeight, GL_RGBA, GL_UNSIGNED_BYTE, framebufferData.data());
      RendererAPI<QGL_3_3>()->glReadPixels(0, 0, framebufferWidth, framebufferHeight, GL_DEPTH_COMPONENT, GL_FLOAT, DepthBufferData.data());
    }
  }
}