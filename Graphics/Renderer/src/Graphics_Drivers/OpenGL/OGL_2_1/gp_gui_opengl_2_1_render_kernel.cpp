#include <exception>

#include "gp_gui_geometry_descriptor.h"
#include "gp_gui_opengl_2_1_render_kernel.h"
#include "gp_gui_opengl_2_1_vertex_array_object.h"
#include "gp_gui_shader_library.h"

#include "gp_gui_communications.h"
#include "gp_gui_events.h"

#include "gp_gui_instrumentation.h"
#include "gp_gui_pixel_utils.h"

#include "graphics_api.hpp"

// #include <glm/gtx/string_cast.hpp>

namespace gridpro_gui
{

using namespace OpenGL_2_1;

    OpenGL_2_1_RenderKernel::OpenGL_2_1_RenderKernel() : Abstract_RenderKernel()
    {
      
    }

    void OpenGL_2_1_RenderKernel::init()
    {
          if(init_flag)
             return;

          if (m_geometry_descriptor == nullptr)
            throw std::runtime_error("Geometry Descriptor is not set");

          GP_TRACE("OpenGL_2_1_RenderKernel::init()");
                    
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

    OpenGL_2_1_RenderKernel::~OpenGL_2_1_RenderKernel()
    {

    }

    /// @brief Load the geometry descriptor (For setting the geometry descriptor)
    void OpenGL_2_1_RenderKernel::load_geometry_descriptor(const std::shared_ptr<GeometryDescriptor> &geometry_descriptor)
    {
          reset();
          m_geometry_descriptor = geometry_descriptor;
          GP_TRACE("OpenGL_2_1_RenderKernel::load_geometry_descriptor() : ", (*m_geometry_descriptor)->get_instance_name());
          init();
    }

    /// @brief Render the geometry in display mode (For rendering the geometry)
    bool OpenGL_2_1_RenderKernel::render_display_mode()
    {
          GLint current_render_mode;
          RendererAPI<QGL_2_1>()->glGetIntegerv(GL_RENDER_MODE, &current_render_mode);
          if(current_render_mode == GL_SELECT) return false;
          
          if(init_flag == false) return false;

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
            {
              (*m_geometry_descriptor)->color.swap((*m_geometry_descriptor)->custom_highlight_color);
            }

            if((*m_geometry_descriptor)->colors_vector().size() == 0)
            {
               use_per_vertex_color = false;
            }
            else
            {
              use_per_vertex_color = true;
            }

            set_blend_state();
            set_depth_test();

            SceneState &scene_state = Event::Publisher::GetInstance()->get_scene_state();

            RendererAPI<QGL_2_1>()->glEnable(GL_COLOR_MATERIAL);

            if(scene_state.enable_lighting == true && (*m_geometry_descriptor)->normals_vector().size() != 0)
            {
              RendererAPI<QGL_2_1>()->glEnable(GL_LIGHT0);
              RendererAPI<QGL_2_1>()->glEnable(GL_LIGHTING);

              glm::vec4 global_ambient(scene_state.LightAmbient, 1.0f);
              RendererAPI<QGL_2_1>()->glLightModelfv(GL_LIGHT_MODEL_AMBIENT, &global_ambient[0]);

              RendererAPI<QGL_2_1>()->glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
              RendererAPI<QGL_2_1>()->glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
              RendererAPI<QGL_2_1>()->glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

              RendererAPI<QGL_2_1>()->glShadeModel(GL_SMOOTH);
              use_per_vertex_color = false;
              enable_lighting = true;
            }
            else
            {
              RendererAPI<QGL_2_1>()->glShadeModel(GL_FLAT);
              RendererAPI<QGL_2_1>()->glDisable(GL_LIGHTING);
            }


            RendererAPI<QGL_2_1>()->glMatrixMode(GL_PROJECTION);
            RendererAPI<QGL_2_1>()->glLoadMatrixf(glm::value_ptr(scene_state.m_projection)); // Load glm projection matrix
    
            RendererAPI<QGL_2_1>()->glMatrixMode(GL_MODELVIEW);
            RendererAPI<QGL_2_1>()->glLoadMatrixf(glm::value_ptr(scene_state.m_model*scene_state.m_view)); // Load glm modelview matrix
  
            if(enable_lighting)
            {

              RendererAPI<QGL_2_1>()->glLightfv(GL_LIGHT0, GL_POSITION, glm::value_ptr(scene_state.LightPosition));
              RendererAPI<QGL_2_1>()->glLightfv(GL_LIGHT0, GL_AMBIENT,  glm::value_ptr(scene_state.LightAmbient));
              RendererAPI<QGL_2_1>()->glLightfv(GL_LIGHT0, GL_SPECULAR, glm::value_ptr(scene_state.LightSpecular));
              RendererAPI<QGL_2_1>()->glLightfv(GL_LIGHT0, GL_DIFFUSE,  glm::value_ptr(scene_state.LightDiffuse));   
              
              glm::vec3 materialAmbient =  glm::vec3(0.3f, 0.3f, 0.3f);
              glm::vec3 materialDiffuse =  glm::vec3(0.3f, 0.3f, 0.3f);
              glm::vec3 materialSpecular = glm::vec3(0.3f, 0.3f, 0.3f);
              float materialShininess = 4.0f; 

              RendererAPI<QGL_2_1>()->glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, glm::value_ptr(materialAmbient));
              RendererAPI<QGL_2_1>()->glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, glm::value_ptr(materialDiffuse));
              RendererAPI<QGL_2_1>()->glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, glm::value_ptr(materialSpecular));
              RendererAPI<QGL_2_1>()->glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);
              
            }

            // Enable if you want to use the texture
            // m_shader->Set1i("textureSampler", *m_texture);
            m_vao->bind();

            //// Draw the geometry in fill mode if wireframe mode is overlay
            if((*m_geometry_descriptor)->get_wireframe_mode_enum() == GL_WIREFRAME_OVERLAY)
            {
                glm::vec4 object_color = glm::make_vec4((*m_geometry_descriptor)->color.get_color().data());
                if(!use_per_vertex_color)
                {
                  RendererAPI<QGL_2_1>()->glColor4f(object_color.r, object_color.g, object_color.b, object_color.a);
                }
                // Draw Call
                execute_draw_command();
                
                //// Draw the in wireframe only or fill mode only based on the rasteriser state
                glm::vec4 wireframe_color = glm::make_vec4((*m_geometry_descriptor)->wireframecolor.get_color().data());
                if(!use_per_vertex_color)
                {
                    RendererAPI<QGL_2_1>()->glColor4f(wireframe_color.r, wireframe_color.g, wireframe_color.b, wireframe_color.a);
                }
                set_rasteriser_state();
                // Draw Call
                execute_draw_command();

                reset_rasteriser_state();
            }
            else if((*m_geometry_descriptor)->get_wireframe_mode_enum() == GL_WIREFRAME_ONLY)
            {
                glm::vec4 wireframe_color = glm::make_vec4((*m_geometry_descriptor)->wireframecolor.get_color().data());
                if (!(use_per_vertex_color))
                {
                  RendererAPI<QGL_2_1>()->glColor4f(wireframe_color.r, wireframe_color.g, wireframe_color.b, wireframe_color.a);
                }
                set_rasteriser_state();
                // Draw Call
                execute_draw_command();

                reset_rasteriser_state();
            }
            else
            {
                glm::vec4 object_color = glm::make_vec4((*m_geometry_descriptor)->color.get_color().data());
                if(!(use_per_vertex_color))
                {
                  RendererAPI<QGL_2_1>()->glDisableClientState(GL_COLOR_ARRAY);
                  RendererAPI<QGL_2_1>()->glColor4f(object_color.r, object_color.g, object_color.b, object_color.a);
                }
                set_rasteriser_state();
                // Draw Call
                execute_draw_command();

                reset_rasteriser_state();
            }
            if((*m_geometry_descriptor)->isNodeManipulationEnabled())
            {
               RendererAPI<QGL_2_1>()->glPointSize(10.0f);
               glm::vec4 object_color(0.0f, 1.0f, 0.5, 1.0f);
               RendererAPI<QGL_2_1>()->glColor4f(object_color.r, object_color.g, object_color.b, object_color.a);
               point_mode_draw();
               RendererAPI<QGL_2_1>()->glPointSize(1.0f);
            }
            // Unbind the all the objects
            // m_texture->unbind();
            m_vao->unbind();

            if(use_custom_highlight_color)
            {
              (*m_geometry_descriptor)->color.swap((*m_geometry_descriptor)->custom_highlight_color);
            }
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
    bool OpenGL_2_1_RenderKernel::render_selection_mode()
    {
          // GLint current_render_mode;
          // RendererAPI<QGL_2_1>()->glGetIntegerv(GL_RENDER_MODE, &current_render_mode);
          // if(current_render_mode == GL_SELECT) return false;

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
             
            SceneState& scene_state = Event::Publisher::GetInstance()->get_scene_state();

            /// Set the shader uniforms
            RendererAPI<QGL_2_1>()->glMatrixMode(GL_PROJECTION);
            RendererAPI<QGL_2_1>()->glLoadMatrixf(glm::value_ptr(scene_state.m_projection)); // Load glm projection matrix
    
            RendererAPI<QGL_2_1>()->glMatrixMode(GL_MODELVIEW);
            RendererAPI<QGL_2_1>()->glLoadMatrixf(glm::value_ptr(scene_state.m_model*scene_state.m_view)); // Load glm modelview matrix
            
            RendererAPI<QGL_2_1>()->glDisable(GL_LIGHTING);
            RendererAPI<QGL_2_1>()->glShadeModel(GL_FLAT);
           
            if (pick_scheme == GL_PICK_GEOMETRY)
            {
                m_vao->set_selection_array_mode(false);
                uint32_t unique_color = m_geometry_descriptor->get_color_id_reserve_start();
                PixelData color = PixelData(unique_color);
                RendererAPI<QGL_2_1>()->glColor4f(color.r_float(), color.g_float(), color.b_float(), 1.0f);
            }
            else if(pick_scheme == GL_PICK_BY_PRIMITIVE || pick_scheme == GL_PICK_BY_VERTEX)
            {
                m_vao->set_selection_array_mode(true);   
                m_vao->generate_unique_color_array();
            } 

            //// Draw the geometry
            m_vao->bind();

            if((*m_geometry_descriptor)->get_wireframe_mode_enum() == GL_WIREFRAME_ONLY)
               set_rasteriser_state();

            if(pick_scheme == GL_PICK_BY_VERTEX)
              RendererAPI<QGL_2_1>()->glPointSize(20.0f);
            
            execute_draw_command(primitive_type);
            

            if((*m_geometry_descriptor)->get_wireframe_mode_enum() == GL_WIREFRAME_ONLY)
               reset_rasteriser_state();

            // Unbind the all the objects
            m_vao->set_selection_array_mode(false); 
            m_vao->unbind();
            GP_TRACE((*m_geometry_descriptor)->get_instance_name(), " : Rendered in Select Mode Sucessfully");
          }

          catch (const std::exception &e)
          {
            std::cerr << e.what() << '\n';
            return false;
          }
          is_in_selection_mode = false;
          return true;
    }

    /// @brief Reset the render kernel (For reinitialization of the kernel with new geometry descriptor)
    void OpenGL_2_1_RenderKernel::reset()
    {
      m_geometry_descriptor.reset();
      m_vao.reset();
      init_flag = false;
    }

    /// @brief Execute the draw command (Just a wrapper for the OpenGL draw commands)
    void OpenGL_2_1_RenderKernel::execute_draw_command(const GLenum &in_primitive_type)
    {
      // Instrumentation::Stopwatch watch("OpenGL_2_1_RenderKernel::DrawCommand");
      GLenum curr_primitive_type = in_primitive_type;

      if(curr_primitive_type == GL_NONE_NULL)
        curr_primitive_type = (*m_geometry_descriptor)->get_primitive_type_enum();

      if(curr_primitive_type == GL_NONE_NULL)
        throw std::runtime_error("Primitive type is not set");
       
      GLenum PickScheme = (*m_geometry_descriptor)->get_pick_scheme_enum();
      
      if((*m_geometry_descriptor)->indices_vector().size() == 0 || (is_in_selection_mode && (PickScheme == GL_PICK_BY_PRIMITIVE || PickScheme == GL_PICK_BY_VERTEX)))
      RendererAPI<QGL_2_1>()->glDrawArrays(curr_primitive_type, 0, (*m_geometry_descriptor)->get_num_vertices());        
      else
      RendererAPI<QGL_2_1>()->glDrawElements(curr_primitive_type,  (*m_geometry_descriptor)->get_num_vertices(), GL_UNSIGNED_INT, (*m_geometry_descriptor)->get_indices_weak_ptr().lock().get()->data());
    }

    void OpenGL_2_1_RenderKernel::point_mode_draw()
    {
      // Round the points to circle
      RendererAPI<QGL_2_1>()->glEnable(GL_POINT_SMOOTH);
      RendererAPI<QGL_2_1>()->glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
      RendererAPI<QGL_2_1>()->glDrawArrays(GL_POINTS, 0, (*m_geometry_descriptor)->positions_vector().size()/3);
      RendererAPI<QGL_2_1>()->glDisable(GL_POINT_SMOOTH);
    }

    void OpenGL_2_1_RenderKernel::set_blend_state()
    {
      SceneState &scene_state = Event::Publisher::GetInstance()->get_scene_state();
      if(scene_state.enable_blending == true)
      {
        if (scene_state.is_blending_enabled == false)
        {
          RendererAPI<QGL_2_1>()->glEnable(GL_BLEND);
          RendererAPI<QGL_2_1>()->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
          scene_state.is_blending_enabled = true;
        }
      }
      else
      {
          RendererAPI<QGL_2_1>()->glDisable(GL_BLEND);
          scene_state.is_blending_enabled = false;
      }
    }

    void OpenGL_2_1_RenderKernel::set_depth_test()
    {
      SceneState &scene_state = Event::Publisher::GetInstance()->get_scene_state();
      if(scene_state.depth_test_enable == true)
      {
        {
          RendererAPI<QGL_2_1>()->glEnable(GL_DEPTH_TEST);
          scene_state.is_depth_test_enabled = true;
        }
      }
      else
      {
          RendererAPI<QGL_2_1>()->glDisable(GL_DEPTH_TEST);
          scene_state.is_depth_test_enabled = false;
      }
    }

    void OpenGL_2_1_RenderKernel::set_rasteriser_state()
    {
      if ((*m_geometry_descriptor)->get_wireframe_mode_enum() != GL_WIREFRAME_NONE)
      {
          RendererAPI<QGL_2_1>()->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
          RendererAPI<QGL_2_1>()->glEnable(GL_POLYGON_OFFSET_FILL);
          RendererAPI<QGL_2_1>()->glPolygonOffset(1.0f, 1.0f);

          RendererAPI<QGL_2_1>()->glEnable (GL_LINE_SMOOTH);
	        RendererAPI<QGL_2_1>()->glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
      }

      if ((*m_geometry_descriptor)->get_primitive_type_enum() == GL_POINTS)
      {
          if(is_in_selection_mode == true)
          {
            RendererAPI<QGL_2_1>()->glPointSize(20.0f);
          }
          else if ((*m_geometry_descriptor)->get_point_size() != 1.0f)
          {
            RendererAPI<QGL_2_1>()->glPointSize((*m_geometry_descriptor)->get_point_size());
          }

          // Round the points to circle
          RendererAPI<QGL_2_1>()->glEnable(GL_POINT_SMOOTH);
          RendererAPI<QGL_2_1>()->glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);          
      }

      else if((*m_geometry_descriptor)->get_primitive_type_enum() == GL_LINES || (*m_geometry_descriptor)->get_primitive_type_enum() == GL_LINE_STRIP ||
              (*m_geometry_descriptor)->get_primitive_type_enum() == GL_LINE_LOOP || (*m_geometry_descriptor)->get_wireframe_mode_enum() != GL_WIREFRAME_NONE)
      {
        float line_width = (*m_geometry_descriptor)->get_line_width();

        if (is_in_selection_mode == true)
            line_width = 20.0f;

        RendererAPI<QGL_2_1>()->glLineWidth(line_width);
        RendererAPI<QGL_2_1>()->glEnable (GL_LINE_SMOOTH);
	      RendererAPI<QGL_2_1>()->glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        RendererAPI<QGL_2_1>()->glDepthFunc(GL_LEQUAL); 
      }
    }

    void OpenGL_2_1_RenderKernel::reset_rasteriser_state()
    {
      if((*m_geometry_descriptor)->get_wireframe_mode_enum() != GL_WIREFRAME_NONE)
      {
        RendererAPI<QGL_2_1>()->glDisable(GL_POLYGON_OFFSET_FILL);
        RendererAPI<QGL_2_1>()->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        RendererAPI<QGL_2_1>()->glDepthFunc(GL_LESS);
      }

      if((*m_geometry_descriptor)->get_primitive_type_enum() == GL_POINTS)
      {
        if((*m_geometry_descriptor)->get_point_size() != 1.0f)
          RendererAPI<QGL_2_1>()->glPointSize(1.0f);
      }
      else if((*m_geometry_descriptor)->get_primitive_type_enum() == GL_LINES || (*m_geometry_descriptor)->get_primitive_type_enum() == GL_LINE_STRIP || (*m_geometry_descriptor)->get_primitive_type_enum() == GL_LINE_LOOP)
      {
        RendererAPI<QGL_2_1>()->glLineWidth(1.0f);
        RendererAPI<QGL_2_1>()->glDepthFunc(GL_LESS);
      }
    }
} // namespace gridpro_gui