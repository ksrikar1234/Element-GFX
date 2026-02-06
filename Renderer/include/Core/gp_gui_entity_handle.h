#ifndef GP_GUI_ENTITY_HANDLE_H
#define GP_GUI_ENTITY_HANDLE_H

#include <string>

#include "ecs.h"


namespace GridPro_GFX
{
    class Scene_Manager;
    
    class Entity_Handle
    {
    public:
         Entity_Handle();
        ~Entity_Handle();
         
         ///@brief Get the component of the entity
         template<typename C>
         C* GetComponent()
         {
           if(entity_ptr == nullptr || (entity_ptr->is_valid() == false))
              throw std::runtime_error("Entity is not valid");

           else if(entity_ptr->has<C>())
              return &entity_ptr->get<C>();
           
           else
              throw std::runtime_error("Entity does not have the component" + std::string(typeid(C).name()));
         
         }

         ///@brief Set the component of the entity
         template<typename C>
         C* SetComponent()
         {
           if(entity_ptr == nullptr || (entity_ptr->is_valid() == false))
              throw std::runtime_error("Entity is not valid");

           try
           {
                if(entity_ptr->has<C>())
                {
                    C &return_component = entity_ptr->get<C>();
                    return &return_component;
                }

                else
                {
                    entity_ptr->set<C>();
                    C &return_component = entity_ptr->get<C>();
                    return &return_component;
                }
           }

           catch (const std::exception& e)
           {
                std::cerr << e.what() << '\n';
                throw e;
           }
         }
         
         template<typename C>
         bool HasComponent()
         {
           if(entity_ptr == nullptr || (entity_ptr->is_valid() == false))
              throw std::runtime_error("Entity is not valid");

           else if(entity_ptr->has<C>())
              return true;
           
           else
              return false;
         }

         bool is_valid() const;
         void destroy();
         const std::string& get_key() const;
         
    private:
        friend class  Scene_Manager;
        std::string   entity_key;
        ecs::Entity*  entity_ptr;
        Scene_Manager* scene_ptr;
    };

} // namespace GridPro_GFX


#endif // GP_GUI_ENTITY_HANDLE_H