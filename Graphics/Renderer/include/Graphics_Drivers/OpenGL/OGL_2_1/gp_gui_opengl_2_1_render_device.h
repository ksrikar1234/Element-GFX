#ifndef GP_GUI_OPENGL_2_1_RENDERING_SYSTEM_H
#define GP_GUI_OPENGL_2_1_RENDERING_SYSTEM_H

#include "ecs.h"
#include "abstract_render_context.hpp"

namespace gridpro_gui
{
  class OpenGL_2_1_RenderDevice : public ecs::System
  {
    public:
      void init() override;
      void update(float layer) override;
      void reset();
      unsigned int get_render_context_id() const { return m_render_context.id(); }
      ~OpenGL_2_1_RenderDevice() override { reset(); }
      private :
      render_context m_render_context;
  };
}

#endif // GP_GUI_RENDERING_SYSTEM_H