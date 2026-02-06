#ifndef GP_GUI_OPENGL_3_3_RENDERING_SYSTEM_H
#define GP_GUI_OPENGL_3_3_RENDERING_SYSTEM_H

#include "ecs.h"
#include "abstract_render_context.hpp"
namespace GridPro_GFX
{
  class OpenGL_3_3_RenderDevice : public ecs::System
  {
    public:
      void init() override;
      void update(float layer) override;
      void reset();
      unsigned int get_render_context_id() const { return m_render_context.id(); }
      ~OpenGL_3_3_RenderDevice() override { printf("Resetting Device\n"); reset(); }
      private :
      render_context m_render_context;
      bool is_initialized;
  };
}

#endif // GP_GUI_RENDERING_SYSTEM_H