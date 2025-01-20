#ifndef GP_GUI_OPENGL_2_1_RENDER_KERNEL_H
#define GP_GUI_OPENGL_2_1_RENDER_KERNEL_H

#include <memory>
#include "graphics_api.hpp"
#include "abstract_render_kernel.hpp"

namespace gridpro_gui
{

    namespace OpenGL_2_1
    {
        class VertexArrayObject;
    }
     
    namespace Event
    {
        class Publisher;
    }
     
    class OpenGL_2_1_RenderKernel : public Abstract_RenderKernel
    {
      public :
        OpenGL_2_1_RenderKernel();

        virtual ~OpenGL_2_1_RenderKernel();

        void load_geometry_descriptor(const std::shared_ptr<GeometryDescriptor>& geometry_descriptor);
        std::shared_ptr<GeometryDescriptor>& get_descriptor() { return m_geometry_descriptor; }

        bool render_display_mode();
        bool render_selection_mode();

        void set_kernel_id(uint32_t kernel_id) { m_kernel_id = kernel_id; }
        uint32_t get_kernel_id() { return m_kernel_id; }

      private :
        void init();
        void reset();
        void execute_draw_command(const GLenum &primitive_type = GL_NONE_NULL);
        void point_mode_draw();
        void set_rasteriser_state();
        void reset_rasteriser_state();
        void set_blend_state();
        void set_depth_test();

        // Member Variables
        std::shared_ptr<OpenGL_2_1::VertexArrayObject> m_vao;
    };
}


#endif // GP_GUI_OPENGL_3_3_RENDER_KERNEL_H