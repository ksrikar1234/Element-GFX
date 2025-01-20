#ifndef _ABSTRACT_RENDER_KERNEL_
#define _ABSTRACT_RENDER_KERNEL_

#include <memory>
#include "gp_gui_typedefs.h"

namespace gridpro_gui
{
class GeometryDescriptor;

  class Abstract_RenderKernel
    {
      public :
      explicit Abstract_RenderKernel()
      {
        init_flag = false;
        m_kernel_id = 0;
        is_in_selection_mode = false;
      }
      virtual ~Abstract_RenderKernel() = default;
    
      virtual void load_geometry_descriptor(const std::shared_ptr<GeometryDescriptor>& geometry_descriptor) = 0;
      std::shared_ptr<GeometryDescriptor>& get_descriptor() { return m_geometry_descriptor; }

      virtual bool render_display_mode() = 0;
      virtual bool render_selection_mode() = 0;

      void set_kernel_id(uint32_t kernel_id) { m_kernel_id = kernel_id; }
      uint32_t get_kernel_id()               { return m_kernel_id;      }
      

      protected :
      
      virtual void init() = 0;
      virtual void reset() = 0;
      virtual void execute_draw_command(const GLenum& primitive_type = GL_NONE_NULL) = 0;
      virtual void point_mode_draw() = 0;
      virtual void set_rasteriser_state() = 0;
      virtual void reset_rasteriser_state() = 0;
      virtual void set_blend_state() = 0;

      // Member Variables
      std::shared_ptr<GeometryDescriptor> m_geometry_descriptor;

      bool init_flag;
      uint32_t m_kernel_id;
      bool is_in_selection_mode;
    };

}    

#endif