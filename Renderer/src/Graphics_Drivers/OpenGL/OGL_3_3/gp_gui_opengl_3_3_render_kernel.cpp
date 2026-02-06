
#include <exception>

#include "gp_gui_geometry_descriptor.h"

#include "gp_gui_opengl_3_3_render_kernel.h"
#include "gp_gui_opengl_3_3_vertex_array_object.h"
#include "gp_gui_opengl_3_3_texture.h"
#include "gp_gui_opengl_3_3_shader.h"
#include "gp_gui_shader_library.h"

#include "gp_gui_communications.h"
#include "gp_gui_events.h"

#include "gp_gui_instrumentation.h"
#include "gp_gui_pixel_utils.h"

#include "graphics_api.hpp"

// #include <glm/gtx/string_cast.hpp>

namespace GridPro_GFX
{

using namespace OpenGL_3_3;

    OpenGL_3_3_RenderKernel::OpenGL_3_3_RenderKernel() : Abstract_RenderKernel()
    {
      
    }

    void OpenGL_3_3_RenderKernel::init()
    {
          if(init_flag)
             return;

          if (m_geometry_descriptor == nullptr)
            throw std::runtime_error("Geometry Descriptor is not set");

          GP_TRACE("OpenGL_3_3_RenderKernel::init()");
                    
          try
          {
            //m_geometry_descriptor->isValid();
            m_vao = std::make_shared<VertexArrayObject>(m_geometry_descriptor.get());
          }

          catch (const std::exception &e)
          {  
            GP_PRINT(e.what());
            return;
          }

          m_geometry_descriptor->clearDirtyFlags();

          init_flag = true;
    }

    OpenGL_3_3_RenderKernel::~OpenGL_3_3_RenderKernel()
    {
        GP_TRACE("OpenGL_3_3_RenderKernel::~OpenGL_3_3_RenderKernel()");
    }

    /// @brief Load the geometry descriptor (For setting the geometry descriptor)
    void OpenGL_3_3_RenderKernel::load_geometry_descriptor(const std::shared_ptr<GeometryDescriptor> &geometry_descriptor)
    {
          reset();
          m_geometry_descriptor = geometry_descriptor;
          init();
    }

    /// @brief Render the geometry in display mode (For rendering the geometry)
    bool OpenGL_3_3_RenderKernel::render_display_mode()
    {
          if(init_flag == false)  return false;
          is_in_selection_mode = false;
          try
          {
            if((*m_geometry_descriptor)->positions_vector().size() == 0) return false;
         
            // Bind the texture
            // m_texture->bind(0);
            bool use_per_vertex_color = false;
            bool enable_lighting = false;
            bool use_custom_highlight_color = (*m_geometry_descriptor)->isUsingCustomHighlightColor();

            if(use_custom_highlight_color)
              (*m_geometry_descriptor)->color.swap((*m_geometry_descriptor)->custom_highlight_color);

            if((*m_geometry_descriptor)->colors_vector().size() == 0)
              m_shader = get_shader("BasicShader");
            else
            {
              m_shader = get_shader("BasicPerVertexColorShader");
              use_per_vertex_color = true;
            }
            set_blend_state();
            set_depth_test();

            SceneState &scene_state = Event::Publisher::GetInstance()->get_scene_state();

            if (scene_state.enable_lighting == true && (*m_geometry_descriptor)->normals_vector().size() != 0)
            {
              m_shader = get_shader("PhongsLightingShader");
              use_per_vertex_color = false;
              enable_lighting = true;
            }

            m_vao->bind();            
            m_shader->bind();

            /// Set the shader uniforms
            m_shader->SetMat4fv("projection", scene_state.m_projection);
            m_shader->SetMat4fv("model", scene_state.m_model);
            m_shader->SetMat4fv("view", scene_state.m_view);

            if (enable_lighting)
            {
              try
              {
                m_shader->SetVec3fv("lightPosition", scene_state.LightPosition);
                m_shader->SetVec3fv("lightAmbient",  scene_state.LightAmbient);
                m_shader->SetVec3fv("lightDiffuse",  scene_state.LightDiffuse);
                m_shader->SetVec3fv("lightSpecular", scene_state.LightSpecular);

                m_shader->SetVec3fv("materialAmbient", glm::vec3(0.4f, 0.4f, 0.4f));
                m_shader->SetVec3fv("materialDiffuse", glm::vec3(0.7f, 0.7f, 0.7f));
                m_shader->SetVec3fv("materialSpecular", glm::vec3(0.6f, 0.6f, 0.6f));
                m_shader->Set1f("materialShininess", 256.0f);
              }
              catch(const std::exception& e)
              {
                std::cerr << e.what() << '\n';
              }
            }
            
            // Enable if you want to use the texture
            // m_shader->Set1i("textureSampler", *m_texture);

           
            //// Draw the geometry in fill mode if wireframe mode is overlay
            if((*m_geometry_descriptor)->get_wireframe_mode_enum() == GL_WIREFRAME_OVERLAY)
            {
                glm::vec4 object_color = glm::make_vec4((*m_geometry_descriptor)->color.get_color().data());
                if(!(use_per_vertex_color))
                  m_shader->SetVec4fv("object_color", object_color);

                // Draw Call
                execute_draw_command();
                
                //// Draw the in wireframe only or fill mode only based on the rasteriser state
                glm::vec4 wireframe_color = glm::make_vec4((*m_geometry_descriptor)->wireframecolor.get_color().data());
                if(!(use_per_vertex_color))
                  m_shader->SetVec4fv("object_color", wireframe_color);

                set_rasteriser_state();
                // Draw Call
                
                execute_draw_command();

                reset_rasteriser_state();
            }
            else if((*m_geometry_descriptor)->get_wireframe_mode_enum() == GL_WIREFRAME_ONLY)
            {
                glm::vec4 wireframe_color = glm::make_vec4((*m_geometry_descriptor)->wireframecolor.get_color().data());
                if (!(use_per_vertex_color))
                  m_shader->SetVec4fv("object_color", wireframe_color);

                set_rasteriser_state();
                // Draw Call
                execute_draw_command();

                reset_rasteriser_state();
            }
            else
            {
                glm::vec4 object_color = glm::make_vec4((*m_geometry_descriptor)->color.get_color().data());
                if(!(use_per_vertex_color))
                   m_shader->SetVec4fv("object_color", object_color);

                set_rasteriser_state();
                // Draw Call
                execute_draw_command();

                reset_rasteriser_state();
            }
               
            // Unbind the all the objects
            // m_texture->unbind();
            if((*m_geometry_descriptor)->isNodeManipulationEnabled())
            {
               RendererAPI<QGL_3_3>()->glPointSize(10.0f);
               point_mode_draw();
               RendererAPI<QGL_3_3>()->glPointSize(1.0f);
            }
          
            m_vao->unbind();
            m_shader->unbind();
            GP_TRACE("Rendered in Display Mode Sucessfully");

            if(use_custom_highlight_color)
              (*m_geometry_descriptor)->color.swap((*m_geometry_descriptor)->custom_highlight_color);
            
            GP_TRACE((*m_geometry_descriptor)->get_instance_name(), " : Kernel Rendered in Display Mode Sucessfully");
          }

          catch (const std::exception &e)
          {
            std::cerr << e.what() << '\n';
            return false;
          }

          return true;
    }

    /// @brief Render the geometry in selection mode (For picking the geometry)
    bool OpenGL_3_3_RenderKernel::render_selection_mode()
    {
          if(init_flag == false) return false;

          is_in_selection_mode = true;

          try
          {
            if((*m_geometry_descriptor)->positions_vector().size() == 0)
               return false;
            
            /// Get the pick information
            GLenum pick_scheme = (*m_geometry_descriptor)->get_pick_scheme_enum();

            if(pick_scheme == GL_PICK_NONE)
               return false;

            GLenum primitive_type = (*m_geometry_descriptor)->get_primitive_type_enum();

            if(pick_scheme == GL_PICK_BY_VERTEX)
               primitive_type = GL_POINTS;

            if(pick_scheme == GL_PICK_BY_PRIMITIVE || pick_scheme == GL_PICK_BY_VERTEX)
              m_shader = get_shader("SelectPrimitiveShader");

            else if(pick_scheme == GL_PICK_GEOMETRY)
              m_shader = get_shader("SelectGeometryShader");

            m_shader->bind();
            
            SceneState &scene_state = Event::Publisher::GetInstance()->get_scene_state();

            /// Set the shader uniforms
            m_shader->SetMat4fv("projection", scene_state.m_projection);
            m_shader->SetMat4fv("model", scene_state.m_model);
            m_shader->SetMat4fv("view", scene_state.m_view);
            
            if(pick_scheme == GL_PICK_BY_PRIMITIVE || pick_scheme == GL_PICK_BY_VERTEX)
                m_shader->Set1i("selection_init_id", m_geometry_descriptor->get_color_id_reserve_start());


            else if(pick_scheme == GL_PICK_GEOMETRY)
            {
                uint32_t unique_color = m_geometry_descriptor->get_color_id_reserve_start();
                PixelData color = PixelData(unique_color);
                glm::vec3 unique_color_vec = glm::vec3(color.r_float(), color.g_float(), color.b_float());
                m_shader->SetVec3fv("selection_init_id", unique_color_vec);
            }

            //// Draw the geometry
            m_vao->bind();

            if((*m_geometry_descriptor)->get_wireframe_mode_enum() == GL_WIREFRAME_ONLY)
               set_rasteriser_state();


            if(pick_scheme == GL_PICK_BY_VERTEX)
              RendererAPI<QGL_3_3>()->glPointSize(20.0f);

            execute_draw_command(primitive_type);

            if((*m_geometry_descriptor)->get_wireframe_mode_enum() == GL_WIREFRAME_ONLY)
               reset_rasteriser_state();

            // Unbind the all the objects
            m_vao->unbind();
            m_shader->unbind();
            GP_TRACE((*m_geometry_descriptor)->get_instance_name(), " : Rendered in Select Mode Sucessfully");
            
          }

          catch (const std::exception &e) {
            std::cerr << e.what() << '\n';
            return false;
          }
          is_in_selection_mode = false;
          return true;
    }

    /// @brief Reset the render kernel (For reinitialization of the kernel with new geometry descriptor)
    void OpenGL_3_3_RenderKernel::reset()
    {
      m_geometry_descriptor.reset();
      m_vao.reset();
      m_shader = nullptr;
      m_texture.reset();
      init_flag = false;
    }

    /// @brief Execute the draw command (Just a wrapper for the OpenGL draw commands)
    void OpenGL_3_3_RenderKernel::execute_draw_command(const GLenum &in_primitive_type)
    {
      // Instrumentation::Stopwatch watch("OpenGL_3_3_RenderKernel::DrawCommand");
      GLenum curr_primitive_type = in_primitive_type;

      if(curr_primitive_type == GL_NONE_NULL)
        curr_primitive_type = (*m_geometry_descriptor)->get_primitive_type_enum();

      if(curr_primitive_type == GL_NONE_NULL)
        throw std::runtime_error("Primitive type is not set");

      if((*m_geometry_descriptor)->indices_vector().size() == 0 || (is_in_selection_mode && (*m_geometry_descriptor)->get_pick_scheme_enum() == GL_PICK_BY_VERTEX))
        RendererAPI<QGL_3_3>()->glDrawArrays(curr_primitive_type, 0, (*m_geometry_descriptor)->get_num_vertices());
      else
        RendererAPI<QGL_3_3>()->glDrawElements(curr_primitive_type,  (*m_geometry_descriptor)->get_num_vertices(), GL_UNSIGNED_INT, nullptr);
    }
    
    /// @brief Draw the geometry in point mode (For rendering the geometry in point mode)
    void OpenGL_3_3_RenderKernel::point_mode_draw()
    {
      RendererAPI<QGL_3_3>()->glDrawArrays(GL_POINTS, 0, (*m_geometry_descriptor)->positions_vector().size()/3);
    }


    void OpenGL_3_3_RenderKernel::set_depth_test()
    {
      SceneState &scene_state = Event::Publisher::GetInstance()->get_scene_state();
      if(scene_state.depth_test_enable == true)
      {
        {
          RendererAPI<QGL_3_3>()->glEnable(GL_DEPTH_TEST);
          scene_state.is_depth_test_enabled = true;
        }
      }
      else
      {
          RendererAPI<QGL_3_3>()->glDisable(GL_DEPTH_TEST);
          scene_state.is_depth_test_enabled = false;
      }
    }

    void OpenGL_3_3_RenderKernel::set_blend_state()
    {
      SceneState &scene_state = Event::Publisher::GetInstance()->get_scene_state();
      if(scene_state.enable_blending == true)
      {
        if(scene_state.is_blending_enabled == false)
        {
          RendererAPI<QGL_3_3>()->glEnable(GL_BLEND);
          RendererAPI<QGL_3_3>()->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
          scene_state.is_blending_enabled = true;
        }
      }
      else
      {
        if(scene_state.is_blending_enabled == true)
          RendererAPI<QGL_3_3>()->glDisable(GL_BLEND);
      }
    }

    void OpenGL_3_3_RenderKernel::set_rasteriser_state()
    {
      if((*m_geometry_descriptor)->get_wireframe_mode_enum() != GL_WIREFRAME_NONE)
      {
          RendererAPI<QGL_3_3>()->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
          RendererAPI<QGL_3_3>()->glEnable(GL_POLYGON_OFFSET_FILL);
          RendererAPI<QGL_3_3>()->glPolygonOffset(0.1, 0.1);
      }

      if((*m_geometry_descriptor)->get_primitive_type_enum() == GL_POINTS)
      {
          if((*m_geometry_descriptor)->get_point_size() != 1.0f)
            RendererAPI<QGL_3_3>()->glPointSize((*m_geometry_descriptor)->get_point_size());
      }

      else if((*m_geometry_descriptor)->get_primitive_type_enum() == GL_LINES || (*m_geometry_descriptor)->get_primitive_type_enum() == GL_LINE_STRIP ||
              (*m_geometry_descriptor)->get_primitive_type_enum() == GL_LINE_LOOP || (*m_geometry_descriptor)->get_wireframe_mode_enum() != GL_WIREFRAME_NONE)
      {
        float line_width = (*m_geometry_descriptor)->get_line_width();

        if (is_in_selection_mode == true)
            line_width = 20.0f;

        RendererAPI<QGL_3_3>()->glLineWidth(line_width);
      }
    }

    void OpenGL_3_3_RenderKernel::reset_rasteriser_state()
    {
      if((*m_geometry_descriptor)->get_wireframe_mode_enum() != GL_WIREFRAME_NONE)
      {
        RendererAPI<QGL_3_3>()->glDisable(GL_POLYGON_OFFSET_FILL);
        RendererAPI<QGL_3_3>()->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      }

      if((*m_geometry_descriptor)->get_primitive_type_enum() == GL_POINTS)
      {
        if((*m_geometry_descriptor)->get_point_size() != 1.0f)
          RendererAPI<QGL_3_3>()->glPointSize(1.0f);
      }
      else if((*m_geometry_descriptor)->get_primitive_type_enum() == GL_LINES || (*m_geometry_descriptor)->get_primitive_type_enum() == GL_LINE_STRIP || (*m_geometry_descriptor)->get_primitive_type_enum() == GL_LINE_LOOP)
      {
        RendererAPI<QGL_3_3>()->glLineWidth(1.0f);
      }
    }
    
    OpenGL_3_3::Shader* OpenGL_3_3_RenderKernel::get_shader(const char *shader_name)
    {
       SceneState &scene_state = Event::Publisher::GetInstance()->get_scene_state();
       return ShaderLibrary<OpenGL_3_3::Shader>::GetShader(std::string(shader_name) + "_" + std::to_string(scene_state.get_render_context_id())).get();
    }
    

} // namespace GridPro_GFX