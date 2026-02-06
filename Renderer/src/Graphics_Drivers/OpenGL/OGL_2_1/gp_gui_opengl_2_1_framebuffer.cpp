
#include "gp_gui_opengl_2_1_framebuffer.h"
#include "graphics_api.hpp"
#include "gp_gui_debug.h"

namespace GridPro_GFX
{
  namespace OpenGL_2_1
  {
    /// @brief Constructor
    framebuffer::framebuffer() : Abstract_Framebuffer()
    {
      scan_mode = ScanMode::LEFT_RIGHT;
    }

    /// @brief Destructor
    framebuffer::~framebuffer() {}

    /// @brief Update the current framebuffer data
    // void framebuffer::update_current_frame_buffer()
    // {
    //   GLint viewport[4];
    //   RendererAPI<QGL_2_1>()->glGetIntegerv(GL_VIEWPORT, viewport);
    //   // Required for QGLWidget
    //   //RendererAPI<QGL_2_1>()->glReadBuffer(GL_FRONT);
    //   RendererAPI<QGL_2_1>()->glReadBuffer(GL_BACK);
    //   framebufferWidth  = viewport[2];
    //   framebufferHeight = viewport[3];
    //   framebufferData.resize(framebufferWidth * framebufferHeight * 4);
    //   DepthBufferData.resize(framebufferData.size() / 4);
    //   RendererAPI<QGL_2_1>()->glPixelStorei(GL_PACK_ALIGNMENT, 1);
    //   RendererAPI<QGL_2_1>()->glReadPixels(0, 0, framebufferWidth, framebufferHeight, GL_RGBA, GL_UNSIGNED_BYTE, framebufferData.data());
    //   RendererAPI<QGL_2_1>()->glReadPixels(0, 0, framebufferWidth, framebufferHeight, GL_DEPTH_COMPONENT, GL_FLOAT, DepthBufferData.data());
    //   RendererAPI<QGL_2_1>()->glFinish();
    // }

    void framebuffer::update_current_frame_buffer()
    {
        GLint viewport[4];
        RendererAPI<QGL_2_1>()->glGetIntegerv(GL_VIEWPORT, viewport);

        framebufferWidth  = viewport[2];
        framebufferHeight = viewport[3];
        framebufferData.resize(framebufferWidth * framebufferHeight * 4);
        DepthBufferData.resize(framebufferWidth * framebufferHeight);

        RendererAPI<QGL_2_1>()->glReadBuffer(GL_BACK);

        RendererAPI<QGL_2_1>()->glPixelStorei(GL_PACK_ALIGNMENT, 1);
        RendererAPI<QGL_2_1>()->glReadPixels(0, 0, framebufferWidth, framebufferHeight, GL_DEPTH_COMPONENT, GL_FLOAT, DepthBufferData.data());
        RendererAPI<QGL_2_1>()->glReadPixels(0, 0, framebufferWidth, framebufferHeight, GL_RGBA, GL_UNSIGNED_BYTE, framebufferData.data());
        RendererAPI<QGL_2_1>()->glFinish();
    }
  }
}