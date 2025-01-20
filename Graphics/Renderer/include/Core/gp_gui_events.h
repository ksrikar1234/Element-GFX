#ifndef _GP_GUI_EVENTS_H_
#define _GP_GUI_EVENTS_H_

#include <glm/glm.hpp>
#include <string>


namespace gridpro_gui
{
class Scene_Manager;
#define HLM_BIT_SHIFT(x) (1 << x)


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//  Enums for Event Types and Event Categories
/////////////////////////////////////////////////////////////////////////

enum class EventType
{
    None = 0,
    WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
    AppTick, AppUpdate, AppRender,
    KeyPressed, KeyReleased, KeyTyped,
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
    PickedEntity
};

enum EventCategory
{
    None = 0,
    EventCategoryApplication    = HLM_BIT_SHIFT(0),
    EventCategoryInput          = HLM_BIT_SHIFT(1),
    EventCategoryKeyboard       = HLM_BIT_SHIFT(2),
    EventCategoryMouse          = HLM_BIT_SHIFT(3),
    EventCategoryMouseButton    = HLM_BIT_SHIFT(4),
    EventCategoryPick           = HLM_BIT_SHIFT(5)
};
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
//  Event Base class and 3 Derived classes. 
//  For KeyBoard, Mouse , PickEvent
/////////////////////////////////////////////////////////////////////////

class BaseEvent
{
public:
    virtual ~BaseEvent() = default;

    bool Handled = false;
    
    virtual EventType GetEventType() const = 0;
    virtual void SetEventType (EventType input_event_type)  = 0;
    virtual const char* GetName() const = 0;
    virtual int GetCategoryFlags() const = 0;
    virtual std::string ToString() const { return GetName(); }

    bool isHandled() { return Handled; }
    bool IsInCategory(EventCategory category)
    {
        return GetCategoryFlags() & category;
    }
};

/// @brief Key Event
/// Key Event has 3 types of events
/// 1. KeyPressed
/// 2. KeyReleased
/// 3. KeyTyped
/// Each of these events has own unique key code and event type

class KeyEvent : public BaseEvent
{
public:
 KeyEvent() : event_type(EventType::None) {}
 KeyEvent(const int keyCode,  EventType input_event) : keyCode(keyCode), event_type(input_event) {}

~KeyEvent() {}

void SetEventType(EventType input_event_type) override { event_type =  input_event_type; }

EventType GetEventType() const override { return event_type; }

const char* GetName() const override   
{ 
   switch(event_type)
         {
            case EventType::KeyPressed:
                 return "KeyPressed";
                 break;
            case EventType::KeyTyped:
                 return "KeyTyped";
                 break;
            case EventType::KeyReleased :
                 return "KeyReleased";
                 break;                                  
            default:
                 return "None";
         }     
}

int GetCategoryFlags() const override 
{
   return EventCategory::EventCategoryInput | EventCategory::EventCategoryKeyboard;
}

int GetKeyCode() const 
{
    return keyCode;
}

std::string ToString() const override 
{
    return GetName() + std::string(" (") + std::to_string(keyCode) + std::string(")");
}

private:
    friend class Scene_Manager;
    EventType event_type;
    int keyCode;
};

/// @brief Mouse Event
// Mouse Event has 4 types of events
// 1. MouseButtonPressed
// 2. MouseButtonReleased
// 3. MouseMoved
// 4. MouseScrolled

// Each of these events has own unique mouse x and y coordinates and event type

class MouseEvent : public BaseEvent 
{
public:
MouseEvent() : event_type(EventType::None) { mouseX = 0; mouseY = 0; }
MouseEvent(const float x, const float y, EventType input_event) : mouseX(x), mouseY(y), event_type(input_event)  {}

void SetEventType(EventType input_event_type) override { event_type =  input_event_type; }

EventType GetEventType() const override { return event_type; }

const char* GetName() const override 
{
   switch(event_type)
         {
            case EventType::MouseButtonPressed :
                 return "MouseButtonPressed";
                 break;
            case EventType::MouseButtonReleased :
                 return "MouseButtonReleased";
                 break;
            case EventType::MouseMoved:
                 return "MouseMoved";
                 break;   
            case EventType::MouseScrolled :
                 return "MouseScrolled";
                 break;                                                  
            default:
                  return "None";
         }  
}

int GetCategoryFlags() const override 
{
  return EventCategory::EventCategoryInput | EventCategory::EventCategoryMouse;
}

float getMouseX() const 
{
   return mouseX;
}

float getMouseY() const 
{
    return mouseY;
}

private:
    friend class Scene_Manager;
    EventType event_type;
    float mouseX;
    float mouseY;
};

/// @brief Pick Event
/// Pick Event is derived from BaseEvent class
/// It has a unique color id and event type
/// The color id is used to identify the entity that was picked
/// The event type is used to identify the type of pick event
/// The pick event can be of 2 types
/// 1. PickedEntity
/// 2. None
/// PickedEntity is used to identify the entity that was picked
/// None is used to identify that no entity was picked
/// The color id is used to identify the entity that was picked
/// SubEntityID is used to identify the sub entity that was picked

class PickEvent : public BaseEvent 
{
public:
PickEvent() : event_type(EventType::None) , picked_color_id(0) , sub_entity_id(0) , entity_id(0), depth(0.0f) , entity_name("NULL_ENTITY") {}
PickEvent(const uint32_t picked_color_id_, EventType input_event) 
            : event_type(input_event), picked_color_id(picked_color_id_), sub_entity_id(0) , entity_id(0), depth(0.0f) , entity_name("NULL_ENTITY")  {}

void SetEventType(EventType input_event_type) override { event_type =  input_event_type; }

EventType GetEventType() const override { return event_type; }

const char* GetName() const override 
{
   switch(event_type)
         {
            case EventType::PickedEntity :
                 return "PickedEntity";
                 break;                                                  
            default:
                return "None";
         }  
}

int GetCategoryFlags() const override 
{
  return EventCategory::EventCategoryInput | EventCategory::EventCategoryPick;
}

void setColorID(const uint32_t id)
{
  this->picked_color_id = id;
}

uint32_t getColorID() const 
{
   return picked_color_id;
}

uint32_t getEntityID() const 
{
   return entity_id;
}

void setEntityID(const uint32_t& input_id)  
{
   entity_id = input_id;
}

void setSubEntityID(const uint32_t& input_id)
{
 this->sub_entity_id = input_id;  
}

uint32_t getSubEntityID()
{
   return this->sub_entity_id; 
}

std::string getEntityKey() const 
{
   return entity_name;
}

void setEntityKey(const std::string& inputname)  
{
   entity_name = inputname;
}

void setDepth(const float& input_depth)
{
   depth = input_depth;
}

float getDepth() const 
{
   return depth;
}

//inline uint32_t get_entity_id()

private:
    friend class Scene_Manager;
    EventType event_type;
    uint32_t entity_id;
    uint32_t sub_entity_id;
    uint32_t picked_color_id;
    float depth;
    std::string entity_name;
};

} // namespace gridpro_gui

#endif // GP_GUI_COMMUNICATIONS_H