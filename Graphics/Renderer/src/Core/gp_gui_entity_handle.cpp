#include "gp_gui_entity_handle.h"
#include "gp_gui_scene.h"
#include "gp_gui_typedefs.h"
#include "gp_gui_forward_structs.h"

namespace gridpro_gui
{

    Entity_Handle::Entity_Handle() : entity_ptr(nullptr), scene_ptr(nullptr)
    {
    
    }

    Entity_Handle::~Entity_Handle()
    {
     
    }

    bool Entity_Handle::is_valid() const
    {
        return entity_ptr->is_valid() && scene_ptr != nullptr;
    }

    void Entity_Handle::destroy()
    {
        if(entity_ptr == nullptr || scene_ptr == nullptr)
        {
            std::cout << "Entity or Scene is not valid" << std::endl;
            return;
        } 

        if(entity_ptr->is_valid())
        {
           entity_ptr->destroy(); 
        }
        
        scene_ptr->remove_entity_from_registry(entity_key);  
        
    }

    const std::string& Entity_Handle::get_key() const
    {
        return entity_key;
    }


} // namespace gridpro_gui