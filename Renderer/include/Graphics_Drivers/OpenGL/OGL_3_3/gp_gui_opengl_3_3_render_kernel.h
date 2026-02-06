#ifndef GP_GUI_OPENGL_3_3_RENDER_KERNEL_H
#define GP_GUI_OPENGL_3_3_RENDER_KERNEL_H

#include <memory>
#include "graphics_api.hpp"
#include "abstract_render_kernel.hpp"

namespace GridPro_GFX
{

    namespace OpenGL_3_3
    {
        class Shader;
        class VertexArrayObject;
        class OpenGLTexture;
    }
     
    namespace Event
    {
        class Publisher;
    }
     
    class OpenGL_3_3_RenderKernel : public Abstract_RenderKernel
    {
      public :
        OpenGL_3_3_RenderKernel();

        virtual ~OpenGL_3_3_RenderKernel();

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
        OpenGL_3_3::Shader* get_shader(const char* shader_name);

        // Member Variables
        OpenGL_3_3::Shader* m_shader;
        std::shared_ptr<OpenGL_3_3::VertexArrayObject> m_vao;
        std::shared_ptr<OpenGL_3_3::OpenGLTexture>     m_texture;
    };
}


#endif // GP_GUI_OPENGL_3_3_RENDER_KERNEL_H