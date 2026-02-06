
#include "gp_gui_communications.h"

#include "gp_gui_scene.h"

#include "gp_gui_opengl_3_3_framebuffer.h"
#include "gp_gui_opengl_2_1_framebuffer.h"

namespace GridPro_GFX
{
  namespace Event
  {
    Publisher::Publisher() : scene_ptr(nullptr)
    {
      PickEvent empty_pick_event;
      KeyEvent empty_key_event;
      MouseEvent empty_mouse_event;

      GlobalMouseEvent.emplace_back(empty_mouse_event);
      GlobalKeyBoardEvent.emplace_back(empty_key_event);
      GlobalPickEvent.emplace_back(empty_pick_event);

      curr_frame_buffer = nullptr;

      m_frame_buffer_3_3 = std::make_shared<OpenGL_3_3::framebuffer>();
      m_frame_buffer_2_1 = std::make_shared<OpenGL_2_1::framebuffer>();

      curr_frame_buffer = m_frame_buffer_2_1.get();
    }

    Abstract_Framebuffer* Publisher::frame_buffer()
    {
      if(scene_ptr->get_scene_state().get_framebuffer_enum() == SceneState::FrameBufferEnum::OpenGL_3_3_BACK_BUFFER)
        curr_frame_buffer = m_frame_buffer_3_3.get();
      else
        curr_frame_buffer = m_frame_buffer_2_1.get();
        
      return (this->curr_frame_buffer);
    }
    
    Abstract_Framebuffer* Publisher::frame_buffer_ogl_3_3()
    {
      return m_frame_buffer_3_3.get();
    }

    Abstract_Framebuffer* Publisher::frame_buffer_ogl_2_1()
    {
      return m_frame_buffer_2_1.get();
    }

    void Publisher::RegisterSubscriber(const std::string &SubscriberID, Subscription *Subscriber)
    {
      SubscribersRegistry[SubscriberID] = Subscriber;
    }

    void Publisher::UnregisterSubscriber(const std::string &SubscriberID)
    {
      SubscribersRegistry.erase(SubscriberID);
    }

    MouseEvent &Publisher::RetrieveMouseEvent()
    {
      return GlobalMouseEvent.front();
    }

    KeyEvent &Publisher::RetrieveKeyBoardEvent()
    {
      return GlobalKeyBoardEvent.front();
    }

    PickEvent &Publisher::RetrievePickEvent()
    {
      return GlobalPickEvent.front();
    }

    /// @brief Get Scene State
    SceneState::RenderMode Publisher::get_render_mode()
    {
      return get_scene_ptr()->get_render_mode();
    }

    SceneState &Publisher::get_scene_state()
    {
      return get_scene_ptr()->get_scene_state();
    }

    void Publisher::set_scene_ptr(Scene_Manager *scene)
    {
      scene_ptr = scene;
    }

    Scene_Manager *Publisher::get_scene_ptr()
    {
      if (scene_ptr == nullptr)
      {
        throw std::runtime_error("Scene Pointer is not valid");
      }
      return scene_ptr;
    }
  }
} // namespace GridPro_GFX