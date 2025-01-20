#ifndef GP_GUI_FORWARD_STRUCTS_H
#define GP_GUI_FORWARD_STRUCTS_H

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdint>

namespace gridpro_gui
{
    class Entity_Handle;
    class Scene_Manager;
    // unique color reservation
    class unique_color_reservation
    { 
      public :
      uint32_t _EntityID_;
      uint32_t _Min_ColorID_, _Max_ColorID_;

      unique_color_reservation() {}

      unique_color_reservation(uint32_t EntityID, uint32_t Min_ColorID, uint32_t Max_ColorID)
      : _EntityID_(EntityID), _Min_ColorID_(Min_ColorID), _Max_ColorID_(Max_ColorID) {}

    };

    struct SceneState 
    {
      SceneState() : m_render_mode(NONE), m_projection(glm::mat4(1.0f)), m_view(glm::mat4(1.0f)), m_model(glm::mat4(1.0f)),  enable_blending(false), is_blending_enabled(false), enable_lighting(false), render_systems_enabled(true)
                     , depth_test_enable(true), is_depth_test_enabled(true), m_render_context_id(0)
      {
        LightAmbient  = glm::vec3(0.3f, 0.3f, 0.3f);
        LightDiffuse  = glm::vec3(0.4f, 0.4f, 0.4f);
        LightSpecular = glm::vec3(1.0f, 1.0f, 1.0f);
        m_driver_enum = OpenGL_2_1;
        cached_is_depth_test_enabled = false;
        cached_depth_test_enabled = false;
      }
     ~SceneState() {}
      enum RenderMode { NONE = 0 , RENDER = 1, SELECT = 2, RENDER_AND_SELECT = 3 }; 
      enum DriverEnum { OpenGL_2_1 = 0, OpenGL_3_3 = 1 };
      enum FrameBufferEnum { OpenGL_2_1_FRONT_BUFFER = 0, OpenGL_3_3_BACK_BUFFER = 1 };

      /// Scene Render Mode
      RenderMode m_render_mode;
       
      uint32_t get_render_context_id() { return m_render_context_id; }
      
      /// Scene Driver Enum
      DriverEnum m_driver_enum;

      /// Scene Framebuffer Enum
      FrameBufferEnum m_framebuffer_enum;
      
      /// Scene MVP
      glm::mat4 m_projection, m_view , m_model;
      glm::mat4 cached_projection, cached_view, cached_model;


      
      bool enable_lighting;  

      /// Scene Light Properties
      glm::vec3 LightPosition;
      glm::vec3 LightAmbient, LightDiffuse, LightSpecular;

      /// Scene Material Properties
      bool enable_blending;
      bool is_blending_enabled;

      bool depth_test_enable;
      bool is_depth_test_enabled;

      bool cached_is_depth_test_enabled;
      bool cached_depth_test_enabled;

      bool  render_systems_enabled;
      uint32_t m_render_context_id;
      
      bool  is_render_systems_enabled()   { return render_systems_enabled; }
      bool  flip_render_systems_switch()  { render_systems_enabled = !render_systems_enabled; return render_systems_enabled; }
   
      bool  set_render_systems_switch(const bool& input_state) { render_systems_enabled = input_state; return render_systems_enabled; }
      bool  get_render_systems_switch() { return render_systems_enabled; }
      
      bool  enable_render_systems()     { render_systems_enabled = true;  return render_systems_enabled;   }
      bool  disable_render_systems()    { render_systems_enabled = false; return render_systems_enabled;   }

      RenderMode get_render_mode()      {  return m_render_mode; }
      void set_render_mode(const RenderMode& in_render_mode) { m_render_mode = in_render_mode; } 

      DriverEnum get_driver_enum()      {  return m_driver_enum; }
      void set_driver_enum(const DriverEnum& in_driver_enum) { m_driver_enum = in_driver_enum; if(in_driver_enum == OpenGL_2_1) m_framebuffer_enum = OpenGL_2_1_FRONT_BUFFER; else m_framebuffer_enum = OpenGL_3_3_BACK_BUFFER; }

      FrameBufferEnum get_framebuffer_enum() { return m_framebuffer_enum; }
      void set_framebuffer_enum(const FrameBufferEnum& in_framebuffer_enum) { m_framebuffer_enum = in_framebuffer_enum; }

      bool get_depth_test() { return depth_test_enable; }
      void set_depth_test(const bool& in_depth_test_enabled) { depth_test_enable = in_depth_test_enabled; }
      
      bool depth_test_enabled() { return is_depth_test_enabled; }

      void cache_state()     
      {
         cached_is_depth_test_enabled = is_depth_test_enabled; cached_depth_test_enabled = depth_test_enable;
         cached_projection = m_projection; cached_view = m_view; cached_model = m_model; 
      }
      
      void set_to_2d_mode()  
      { 
        cache_state(); m_projection = glm::mat4(1.0f); m_view = glm::mat4(1.0f); m_model = glm::mat4(1.0f); 
        is_depth_test_enabled = true; depth_test_enable = false;
      }

      void set_to_3d_mode()   
      {
         m_projection = cached_projection; m_view = cached_view; m_model = cached_model; 
         is_depth_test_enabled = cached_is_depth_test_enabled; depth_test_enable = cached_depth_test_enabled; 
      }
      
     };

    class commit_component
    {
      public :
      commit_component() : commit_flag(true), render_flag(false), render_select_flag(false), m_layer_id(1.0f) {}
     ~commit_component() {}
     
      commit_component(const commit_component& cc)
      {
        commit_flag = cc.commit_flag;
        render_flag = cc.render_flag;
        render_select_flag = cc.render_select_flag;
        m_layer_id = cc.m_layer_id;

      }

      const commit_component& operator=(const commit_component& cc)
      {
        commit_flag = cc.commit_flag;
        render_flag = cc.render_flag;
        render_select_flag = cc.render_select_flag;
        m_layer_id = cc.m_layer_id;
        return *this;
      }

      bool is_committed()                                     { return commit_flag; }
      commit_component* set_commit_flag(const bool& in_flag)  { this->commit_flag = in_flag; return this;}

      bool is_rendered_in_display_mode()                      { return render_flag; }
      void set_rendered_in_display_mode(const bool& in_flag)  { this->render_flag = in_flag; }

      bool is_rendered_in_select_mode()                       { return render_select_flag; }
      void set_rendered_in_select_mode(const bool& in_flag)   { this->render_select_flag = in_flag; }
    
      // commit all means the entity is committed to be rendered
      void commit()    { commit_flag = true; render_flag = false; render_select_flag = false; }

      // uncommit all means the entity is uncommitted to be not rendered
      void uncommit()  { commit_flag = false; render_flag = true; render_select_flag = true; }
      
      float layer_id()                             { return m_layer_id; }
      commit_component*  set_layer_id(const float& in_layer_id) { m_layer_id = in_layer_id; return this; }
      private :
      // Primary Render Resource commit flag 
      bool commit_flag = false;
      // Flags for OpenGL 2.1 Renderer System to avoid redundant rendering
      bool render_flag = false;
      bool render_select_flag = false;

      float m_layer_id;
    };

    struct tag_component 
    {
      tag_component(const std::string& input) : TagName(input), Tag(NONE) {}
     ~tag_component() {}
      std::string TagName;
      enum TAG { NONE, GP_ARBITRARY, GP_CAD, GP_TOPOLOGY, GP_GRID };
      TAG Tag;
      void set_tag_name(const TAG& in_tag) { this->Tag = in_tag; }
      const std::string& tag_name() { return TagName; }
      std::string& tag_name_ref()   { return TagName; }
    };


} // namespace gridpro_gui
#endif // GP_GUI_FORWARD_STRUCTS_H    
