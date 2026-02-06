#ifndef _GP_GUI_COMMUNICATIONS_H_
#define _GP_GUI_COMMUNICATIONS_H_

#include <unordered_map>
#include <deque>
#include <string>
#include <memory>

#include "gp_gui_forward_structs.h"
#include "gp_gui_events.h"

namespace GridPro_GFX
{
/// @brief Publisher and Subscriber classes
/// Publisher is a singleton class that is used to publish events
/// It has a registry of subscribers that are interested in the events
/// It is the only class that can publish events
/// It is used as primary communication between the scene and the user
/// It has a pointer to the scene
/// It has a pointer to the selection buffer
/// It has a deque of mouse events, keyboard events and pick events.
/// It has a notifier that is used to check if the event has been updated

class Scene_Manager;

class Abstract_Framebuffer;

namespace OpenGL_3_3
{
  class framebuffer;
}

namespace OpenGL_2_1
{
  class framebuffer;
}



namespace Event {

class Subscription;

class Publisher {
public:
   
    static Publisher* GetInstance()
    {
        static Publisher SingletonPublisher;
        return &SingletonPublisher;
    }

    Abstract_Framebuffer* frame_buffer();

    Abstract_Framebuffer* frame_buffer_ogl_3_3();

    Abstract_Framebuffer* frame_buffer_ogl_2_1();
    
    void set_scene_ptr(Scene_Manager* scene_ptr); 

    Scene_Manager* get_scene_ptr();
    
    void RegisterSubscriber(const std::string& SubscriberID, Subscription* Subscriber);

    void UnregisterSubscriber(const std::string& SubscriberID);
   
    MouseEvent& RetrieveMouseEvent();

    KeyEvent& RetrieveKeyBoardEvent();

    PickEvent& RetrievePickEvent();

    public:

    std::unordered_map<std::string, Subscription*> SubscribersRegistry;
    std::unordered_map<uint32_t, std::string>*     EntityIdxKeyMapRegistry;

    private :
    friend class Scene_Manager;
    Scene_Manager* scene_ptr; 
    Publisher();

    Abstract_Framebuffer* curr_frame_buffer;

    std::shared_ptr<OpenGL_3_3::framebuffer> m_frame_buffer_3_3;
    std::shared_ptr<OpenGL_2_1::framebuffer> m_frame_buffer_2_1;
    
    public :
    std::deque<MouseEvent> GlobalMouseEvent;
    std::deque<KeyEvent>   GlobalKeyBoardEvent; 
    std::deque<PickEvent>  GlobalPickEvent;

    public :
    bool notifier;
    /// @brief Updates Notifiers of subscribptions
    inline void updateNotifiers(); 
    /// @brief Get Scene State
    SceneState::RenderMode get_render_mode(); 
    SceneState& get_scene_state();
    
};


/// @brief Subscription class
/// Subscription is a class that is used to subscribe to events
/// It is used to get the events from the publisher
/// It has a notifier that is used to check if the event has been updated
/// It has a unique subscriber name that is used to identify the subscriber
/// It has a pointer to the publisher instance
/// It has a pointer to the last mouse event, keyboard event and pick event

class Subscription 
{
public:
Subscription(const std::string SubcriberName) : SubcriberUserID(SubcriberName) 
{ 
    PublisherInstance = Publisher::GetInstance();
    PublisherInstance->RegisterSubscriber(SubcriberName, this); 
    notifier = true; 
}

~Subscription() 
{
  PublisherInstance->UnregisterSubscriber(SubcriberUserID);
}

MouseEvent& getMouseEvent()
{ 
    if(isNotifierUpdated()) 
      {
        last_mouse_event = PublisherInstance->RetrieveMouseEvent();
        return PublisherInstance->RetrieveMouseEvent();
      }
    last_mouse_event.SetEventType(EventType::None);  
    return last_mouse_event; 
}

KeyEvent& getKeyBoardEvent()
{
    if(isNotifierUpdated()) 
      {
        last_keyboard_event = PublisherInstance->RetrieveKeyBoardEvent();
        return PublisherInstance->RetrieveKeyBoardEvent();
      }

    last_keyboard_event.SetEventType(EventType::None);
    return last_keyboard_event; 

}

PickEvent& getPickEvent()
{
    if(isNotifierUpdated()) 
      {
        last_pick_event = PublisherInstance->RetrievePickEvent();
        return PublisherInstance->RetrievePickEvent();
      }
    last_pick_event.SetEventType(EventType::None);
    return last_pick_event; 

}

bool isNotifierUpdated()
{
    return notifier;
}

private : 
friend class Publisher;
Publisher* PublisherInstance;
std::string SubcriberUserID;
bool notifier;
MouseEvent   last_mouse_event;
KeyEvent     last_keyboard_event;
PickEvent    last_pick_event;
std::string  notification_message;

};

inline void Publisher::updateNotifiers() 
{
    for(std::unordered_map<std::string, Subscription*>::iterator it = SubscribersRegistry.begin(); it != SubscribersRegistry.end(); ++it)
           it->second->notifier = true;
}
 

} // namespace Event

} // namespace GridPro_GFX
#endif // GP_GUI_COMMUNICATIONS_H    