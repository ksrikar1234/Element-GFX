

#include "gp_gui_entity_handle.h" // Warning : This has Circular Dependency with gp_gui_scene.h
#include "gp_gui_scene.h"

#include "gp_gui_geometry_descriptor.h"

#include "gp_gui_opengl_3_3_render_device.h"
#include "gp_gui_opengl_3_3_render_kernel.h"

#include "gp_gui_opengl_2_1_render_device.h"
#include "gp_gui_opengl_2_1_render_kernel.h"

#include "abstract_frame_buffer.hpp"

#include "gp_gui_communications.h"
#include "gp_gui_debug.h"


namespace GridPro_GFX
{

    Scene_Manager::Scene_Manager() : RenderSystemsManager(RenderableEntitiesManager), PublisherInstance(Event::Publisher::GetInstance()) , last_color_id(0)
    {
        // Register the Scene with the Publisher
        // Critical ! Do not remove this line  !!!
        PublisherInstance->set_scene_ptr(this);
        initialize_render_devices(); 
        need_to_update_color_reservations = false;
    }

    bool Scene_Manager::initialize_render_devices()
    {
        /// @note Add Compatible RenderDevices to the RenderSystemsManager

        RenderSystemsManager.add<OpenGL_2_1_RenderDevice>();
        //RenderSystemsManager.add<OpenGL_3_3_RenderDevice>();
        
        try
        {
            has_a_valid_render_device = true;
            RenderSystemsManager.init();
        }

        catch (std::vector<std::exception> &e_list)
        {
            GP_ERROR("Error in Render Device Initialization");

            for (auto &e : e_list)
            {
                std::cerr << e.what() << '\n';
            }

            if(RenderSystemsManager.count() == 0)
            {
                GP_ERROR("No Compatible Render Device Found");
                has_a_valid_render_device = false;
                throw e_list;
            }
        }
       

        if(RenderSystemsManager.has<OpenGL_2_1_RenderDevice>())
        {
            m_scene_state_obj.m_driver_enum = SceneState::DriverEnum::OpenGL_2_1;
            m_scene_state_obj.m_render_context_id = RenderSystemsManager.get<OpenGL_2_1_RenderDevice>().get_render_context_id();
        }
        else if(RenderSystemsManager.has<OpenGL_3_3_RenderDevice>())
        {
            m_scene_state_obj.m_driver_enum = SceneState::DriverEnum::OpenGL_3_3;
            m_scene_state_obj.m_render_context_id = RenderSystemsManager.get<OpenGL_3_3_RenderDevice>().get_render_context_id();
        }
        else
        {
            GP_ERROR("No Compatible Render Device Found");
        }
         
        return has_a_valid_render_device;
    }

    bool Scene_Manager::switch_driver(const GLenum& input_driver)
    {
        need_to_update_color_reservations = true;
          
        if(input_driver == GL_DRIVER_OPENGL_2_1)
        {
            GP_PRINT("Switching to OpenGL_2_1");
            if(m_scene_state_obj.m_driver_enum == SceneState::DriverEnum::OpenGL_2_1)
            {
              has_a_valid_render_device = true;
              return true;
            }
            if(RenderSystemsManager.has<OpenGL_2_1_RenderDevice>())
            {
                if(RenderSystemsManager.has<OpenGL_3_3_RenderDevice>())
                {
                    GP_PRINT("Removing OpenGL_3_3_RenderDevice");
                    RenderSystemsManager.get<OpenGL_3_3_RenderDevice>().reset();
                    RenderSystemsManager.remove<OpenGL_3_3_RenderDevice>();
                }
                m_scene_state_obj.m_driver_enum = SceneState::DriverEnum::OpenGL_2_1;
                m_scene_state_obj.m_render_context_id = RenderSystemsManager.get<OpenGL_2_1_RenderDevice>().get_render_context_id();
            }
            else
            {
                RenderSystemsManager.add<OpenGL_2_1_RenderDevice>();
                try
                {  
                    GP_PRINT("Trying toRemoving OpenGL_3_3_RenderDevice");
                    if(RenderSystemsManager.has<OpenGL_3_3_RenderDevice>())
                    {
                        GP_PRINT("Removing OpenGL_3_3_RenderDevice");
                        RenderSystemsManager.get<OpenGL_3_3_RenderDevice>().reset();
                        RenderSystemsManager.remove<OpenGL_3_3_RenderDevice>();
                    }
                    m_scene_state_obj.m_driver_enum = SceneState::DriverEnum::OpenGL_2_1;
                    m_scene_state_obj.m_render_context_id = RenderSystemsManager.get<OpenGL_2_1_RenderDevice>().get_render_context_id();
                }
                catch (std::vector<std::exception>& e_list)
                {
                    GP_ERROR("Error in Render Device Initialization");

                    for (auto &e : e_list)
                    {
                        std::cerr << e.what() << '\n';
                    }

                    if(RenderSystemsManager.count() == 0)
                    {
                        GP_ERROR("No Compatible Render Device Found");
                        has_a_valid_render_device = false;
                        return false;
                    }
                }
            }
        }

        else if(input_driver == GL_DRIVER_OPENGL_3_3)
        {
            GP_PRINT("Switching to OpenGL_3_3");
            if(m_scene_state_obj.m_driver_enum == SceneState::DriverEnum::OpenGL_3_3)
            {
              has_a_valid_render_device = true;
              return true;
            }

            if(RenderSystemsManager.has<OpenGL_3_3_RenderDevice>())
            {
                GP_PRINT("Removing OpenGL_2_1_RenderDevice");
                RenderSystemsManager.has<OpenGL_2_1_RenderDevice>() ? RenderSystemsManager.remove<OpenGL_2_1_RenderDevice>() : void();
                m_scene_state_obj.m_driver_enum = SceneState::DriverEnum::OpenGL_3_3;
                m_scene_state_obj.m_render_context_id = RenderSystemsManager.get<OpenGL_3_3_RenderDevice>().get_render_context_id();
            }
            else
            {
                RenderSystemsManager.add<OpenGL_3_3_RenderDevice>();
                try
                {
                    GP_PRINT("Removing OpenGL_2_1_RenderDevice");
                    GP_PRINT("Initializing OpenGL_3_3_RenderDevice");
                    RenderSystemsManager.has<OpenGL_2_1_RenderDevice>() ? RenderSystemsManager.remove<OpenGL_2_1_RenderDevice>() : void();
                    RenderSystemsManager.init();
                    m_scene_state_obj.m_driver_enum = SceneState::DriverEnum::OpenGL_3_3;
                    m_scene_state_obj.m_render_context_id = RenderSystemsManager.get<OpenGL_3_3_RenderDevice>().get_render_context_id();
                }
                catch (std::vector<std::exception> &e_list)
                {
                    GP_ERROR("Error in Render Device Initialization");

                    for (auto &e : e_list)
                    {
                        std::cerr << e.what() << '\n';
                    }

                    if(RenderSystemsManager.count() == 0)
                    {
                        GP_ERROR("No Compatible Render Device Found");
                        has_a_valid_render_device = false;
                        return false;
                    }
                }
            }
        }
        else
        {
            GP_ERROR("Invalid Driver Enum");
            return false;
        }   
        
        struct entity_details
        {
            std::string entity_name;
            float layer_id;
            std::shared_ptr<GeometryDescriptor> geometry_descriptor;
        };
        
        last_color_id = 0;
        std::vector<entity_details> entity_details_list;

        for(auto& EntityNames : SceneEntityRegistry)
        {
            Entity_Handle entt_handle = get_entity(EntityNames.first);
            std::shared_ptr<GeometryDescriptor> curr_geometry_descriptor;
            if(entt_handle.HasComponent<OpenGL_3_3_RenderKernel>())
            {
                curr_geometry_descriptor = entt_handle.GetComponent<OpenGL_3_3_RenderKernel>()->get_descriptor();
            }
            else if(entt_handle.HasComponent<OpenGL_2_1_RenderKernel>())
            {
                curr_geometry_descriptor = entt_handle.GetComponent<OpenGL_2_1_RenderKernel>()->get_descriptor();
            }
            else
            {
                GP_ERROR("No Render Kernel Found");
                return false;
            }
            entity_details_list.push_back({ EntityNames.first, entt_handle.GetComponent<commit_component>()->layer_id(), curr_geometry_descriptor});
        }

        reset_scene_registry();
         
        
        for(auto& entity : entity_details_list)
        {
            commit_geometry(entity.entity_name, entity.layer_id, entity.geometry_descriptor);
        }
        printf("Switched Driver to : %d\n", input_driver);
        has_a_valid_render_device = true;
        return true;
    }

    bool Scene_Manager::has_render_device() const
    {
        return has_a_valid_render_device;
    }

    Scene_Manager::~Scene_Manager()
    {

    }

    /// @brief Update the scene
    /// @param layer
    /// @details  Use this function to update the scene
    void Scene_Manager::update(const float &layer)
    {
        GP_PLAIN_PRINT("\n+--------------------------------------+\n            Scene Update Started\n---------------------------------------+\n");
     
        PublisherInstance->set_scene_ptr(this);
     
        Event::Subscription scene_subscription("scene");
        if (has_render_device() == false || !(get_scene_state().is_render_systems_enabled()))
        {
            scene_subscription.getPickEvent().setColorID(0);
            scene_subscription.getPickEvent().SetEventType(EventType::None);
            GP_TRACE("Returning from Scene Update as Render Systems are Disabled");
            return;
        }
        
        else
        {
           if(layer == GL_LAYER_PICKABLE) update_color_reservations();
           
           RenderSystemsManager.update(layer);
        }

        if (layer == GL_LAYER_PICKABLE)
        {
            const uint32_t color_id = scene_subscription.getPickEvent().getColorID();
            if ((color_id) != 0 && color_id <= last_color_id)
            {
                scene_subscription.getPickEvent().setEntityKey(EntityIdxKeyMapRegistry[get_actual_id(color_id)]);
                scene_subscription.getPickEvent().setEntityID(get_actual_id(color_id));
                scene_subscription.getPickEvent().setSubEntityID(color_id - (unique_colr_reservations[get_actual_id(color_id)]._Min_ColorID_));
            }
        }

        GP_PLAIN_PRINT("\n+--------------------------------------+\n            Scene Update Ended\n---------------------------------------+\n");
    }
    
    std::vector<std::pair<std::string, uint32_t>> Scene_Manager::pick_matrix(const float &center_x, const float &center_y, const float &width, const float &height)
    {
        std::vector<std::pair<std::string, uint32_t>> picked_entities;
        if (has_render_device() == false)
        {
            return picked_entities;
        }
        
        std::vector<uint32_t> picked_color_ids = PublisherInstance->frame_buffer()->pick_matrix(center_x, center_y, width, height);

        for (auto color_id : picked_color_ids)
        {
            if (color_id != 0 && color_id <= last_color_id)
            {
                picked_entities.emplace_back(EntityIdxKeyMapRegistry[get_actual_id(color_id)], color_id - (unique_colr_reservations[get_actual_id(color_id)]._Min_ColorID_));
            }
        }
        
        return picked_entities;
    }
    
    std::vector<std::pair<std::string, uint32_t>> Scene_Manager::pick_polygon(const std::vector<float>& polygon_points)
    {
        std::vector<std::pair<std::string, uint32_t>> picked_entities;
        if (has_render_device() == false)
        {
            return picked_entities;
        }
        
        std::vector<uint32_t> picked_color_ids = PublisherInstance->frame_buffer()->scanline_polygon(polygon_points);

        for (auto color_id : picked_color_ids)
        {
            if (color_id != 0 && color_id <= last_color_id)
            {
                picked_entities.emplace_back(EntityIdxKeyMapRegistry[get_actual_id(color_id)], color_id - (unique_colr_reservations[get_actual_id(color_id)]._Min_ColorID_));
            }
        }
        
        return picked_entities;
    }

    void Scene_Manager::update_mouse_event(const float &x, const float &y)
    {
        MouseEvent mouse_event;
        mouse_event.mouseX = x;
        mouse_event.mouseY = y;

        PublisherInstance->GlobalMouseEvent.emplace_back(mouse_event);

        /// @brief Keep track of last 6 mouse events
        if (PublisherInstance->GlobalMouseEvent.size() > 6)
        {
            PublisherInstance->GlobalMouseEvent.pop_back();
        }
        
        if(has_render_device() == false )
        {
            return;
        }

        /// @brief  Get the pick event and update it
        Event::Subscription scene_subscription("scene");
        uint32_t color_id = PublisherInstance->frame_buffer()->color_id_at(x, y);
        float depth = PublisherInstance->frame_buffer()->depth_at(x, y);

        scene_subscription.getPickEvent().setColorID(0);
        scene_subscription.getPickEvent().SetEventType(EventType::None);
        scene_subscription.getPickEvent().setEntityKey("NULL_ENTITY");
        scene_subscription.getPickEvent().setDepth(depth);

        if (!get_scene_state().is_render_systems_enabled())
        {
            return;
        }

        if ((color_id) != 0 && color_id <= last_color_id)
        {
            scene_subscription.getPickEvent().setColorID(color_id);
            scene_subscription.getPickEvent().SetEventType(EventType::PickedEntity);
            scene_subscription.getPickEvent().setEntityKey(EntityIdxKeyMapRegistry[get_actual_id(color_id)]);
            scene_subscription.getPickEvent().setEntityID(get_actual_id(color_id));
            scene_subscription.getPickEvent().setSubEntityID(color_id - (unique_colr_reservations[get_actual_id(color_id)]._Min_ColorID_));
            scene_subscription.getPickEvent().setDepth(depth);
        }
    }

    /// @brief Check if the entity exists in the scene
    /// @param entity_key
    /// @return bool
    /// @details  Use this function to check if the entity exists in the scene
    bool Scene_Manager::has_entity(const std::string &entity_key)
    {
        std::unordered_map<std::string, uint32_t>::iterator it = this->SceneEntityRegistry.find(entity_key);
        if (it != SceneEntityRegistry.end())
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    /// @brief Get the entity object from the scene
    /// @param entity_key
    /// @return Entity_Handle
    /// @note If the entity with the given key does not exist, it will be created
    /// @note If the entity with the given key exists, it will be retrieved
    /// @details  Use this function to get the entity object from the scene
    Entity_Handle Scene_Manager::get_entity(const std::string &entity_key)
    {
        // Check if key exists
        std::unordered_map<std::string, uint32_t>::iterator it = this->SceneEntityRegistry.find(entity_key);

        // If key Exists Retrieve it
        if (it != SceneEntityRegistry.end())
        {
            Entity_Handle entt_handle;
            entt_handle.entity_ptr = &Entity_DataBase[SceneEntityRegistry[entity_key]];
            entt_handle.scene_ptr = this;
            entt_handle.entity_key = entity_key;
            return entt_handle;
        }
        // Else Create a new one
        else
        {
            GP_TRACE("Creating Entity : ", entity_key);
            GP_TRACE("Total Entity Count : ", (Entity_DataBase.size()));

            uint32_t curr_assign_id = (Entity_DataBase.size());

            // Register the Entity key-idx pairs
            SceneEntityRegistry[entity_key] = curr_assign_id; /// store Entity key index pair in registry
            EntityIdxKeyMapRegistry[curr_assign_id] = entity_key;

            // Create and store entity in the entity deque
            Entity_DataBase.emplace_back(this->RenderableEntitiesManager.create());

            // Load a Render Kernel based on avaliable devices
            if(RenderSystemsManager.has<OpenGL_3_3_RenderDevice>())
            {  
              Entity_DataBase.back().add<OpenGL_3_3_RenderKernel>().set_kernel_id(curr_assign_id); 
            }
            else if(RenderSystemsManager.has<OpenGL_2_1_RenderDevice>())
            {  
              Entity_DataBase.back().add<OpenGL_2_1_RenderKernel>().set_kernel_id(curr_assign_id);
            }
            else
            {
                GP_ERROR("Warning : No Render Kernels found !!!");
            }
            // Add a Commit Component to the entity

            Entity_DataBase.back().add<commit_component>();

            // Add a entity tag component to the entity
            Entity_DataBase.back().add<tag_component>(entity_key);

            // Create a indirect EntityHandle and return it
            Entity_Handle entt_handle;
            entt_handle.entity_ptr = &(Entity_DataBase.back());
            entt_handle.scene_ptr = this;
            entt_handle.entity_key = entity_key;

            GP_TRACE("Success in Entity Creation : ", entity_key);

            return entt_handle;
        }
    }

    /// @brief Load a geometry descriptor
    /// @param geometry_descriptor
    /// @details  Use this function to load a geometry descriptor into the Appropriate Render Device kernel
    void Scene_Manager::commit_geometry(const std::string &in_name, const float &in_layer_id, const std::shared_ptr<GeometryDescriptor> &geometry_descriptor)
    {
        GP_TRACE("Committing Geometry : ", in_name, " to the Scene");
        Entity_Handle entt_handle = get_entity(in_name);

        if(has_a_valid_render_device)
        {
            if (RenderSystemsManager.has<OpenGL_3_3_RenderDevice>())
            {
                entt_handle.GetComponent<OpenGL_3_3_RenderKernel>()->load_geometry_descriptor(geometry_descriptor);
            }
            else if (RenderSystemsManager.has<OpenGL_2_1_RenderDevice>())
            {
                GP_TRACE("Loading Geometry Descriptor to OpenGL_2_1_RenderKernel");
                entt_handle.GetComponent<OpenGL_2_1_RenderKernel>()->load_geometry_descriptor(geometry_descriptor);
            }
            
            entt_handle.GetComponent<GridPro_GFX::commit_component>()->set_layer_id(in_layer_id)->commit();
            GLenum pick_scheme = (*geometry_descriptor)->get_pick_scheme_enum();
            if(pick_scheme != 0)
            {
                need_to_update_color_reservations = true;
            }
        }
        else
        {
            GP_ERROR("No Compatible Render Device Found");
        }
    }
    
    /// @brief Get the geometry descriptor
    /// @param entity_name
    /// @details  Use this function to Get a geometry descriptor from the Appropriate Render Device kernel
    std::shared_ptr<GeometryDescriptor>& Scene_Manager::get_geometry(const std::string& in_name)
    {
        if(has_entity(in_name))
        {
            Entity_Handle entt_handle = get_entity(in_name);

            if(RenderSystemsManager.has<OpenGL_3_3_RenderDevice>())
            {
                return entt_handle.GetComponent<OpenGL_3_3_RenderKernel>()->get_descriptor();
            }
            else if(RenderSystemsManager.has<OpenGL_2_1_RenderDevice>())
            {
                return entt_handle.GetComponent<OpenGL_2_1_RenderKernel>()->get_descriptor();
            }
            else
            {
                GP_ERROR("No RenderDevice found when retrieving the Geometry the Geometry descriptor");
                throw(std::runtime_error("No RenderDevice found when retrieving the Geometry descriptor"));
            }
        }
        else
        {
            GP_ERROR("No Entity With Name : ", in_name, "found while retrieving the geometry");
            throw(std::runtime_error("No Entity With Name : " + in_name + "found while retrieving the geometry"));
        }
    }    

    /// @details Scene_Manager::remove_entity_from_registry()
    /// @brief Remove the entity from the scene
    /// @param entity_key
    /// @return bool
    /// @details  Use this function to remove the entity from the scene
    bool Scene_Manager::remove_entity_from_registry(const std::string &entity_key)
    {
        // Reassigning IDs
        std::unordered_map<std::string, uint32_t>::iterator idx = SceneEntityRegistry.begin();

        while (idx != SceneEntityRegistry.end())
        {
            if (idx->second > SceneEntityRegistry[entity_key])
            {
                --(idx->second);
            }
            ++idx;
        }

        std::deque<ecs::Entity>::iterator it = Entity_DataBase.begin();

        while ((*it != (Entity_DataBase[SceneEntityRegistry[entity_key]])))
        {
            if (it == Entity_DataBase.end())
                break;
            ++it;
        }

        if (it != Entity_DataBase.end())
        {
            Entity_DataBase.erase(it);
            EntityIdxKeyMapRegistry.erase(SceneEntityRegistry[entity_key]);
            unique_colr_reservations.erase(SceneEntityRegistry[entity_key]);
            SceneEntityRegistry.erase(entity_key);
            return true;
        }

        return false;
    }

    /// @brief Destroy the entity from the scene
    /// @param entity_key
    /// @return bool
    /// @details  Use this function to destroy the entity from the scene
    bool Scene_Manager::destroy_entity(const std::string &entity_key)
    {
        if (has_entity(entity_key))
        {
            Entity_Handle entt_handle = get_entity(entity_key);
            
            GeometryDescriptor* geometry_descriptor = entt_handle.GetComponent<OpenGL_3_3_RenderKernel>()->get_descriptor().get();

            if((*geometry_descriptor)->get_pick_scheme_enum() != 0); 
            {
               need_to_update_color_reservations = true;
            }
            entt_handle.destroy();

            return true;
        }
        return false;
    }

    /// @brief Destroy all entities in the given layer
    /// @param layer
    /// @details  Use this function to destroy all entities in the given layer
    void Scene_Manager::destroy_entities_in_layer(const float &layer)
    {
        for (auto entity : RenderableEntitiesManager.with<commit_component>())
        {
            if (entity.get<commit_component>().layer_id() == layer)
            {
                std::string entity_name(std::move(entity.get<tag_component>().tag_name_ref()));
                
                GeometryDescriptor* geometry_descriptor;

                if(RenderSystemsManager.has<OpenGL_3_3_RenderDevice>())
                {
                    geometry_descriptor = entity.get<OpenGL_3_3_RenderKernel>().get_descriptor().get();
                }
                else if(RenderSystemsManager.has<OpenGL_2_1_RenderDevice>())
                {
                    geometry_descriptor = entity.get<OpenGL_2_1_RenderKernel>().get_descriptor().get();
                }
                else
                {
                    GP_ERROR("No RenderDevice found when destroying the entity");
                    throw std::runtime_error("No RenderDevice found when destroying the entity");
                }

                if ((*geometry_descriptor)->get_pick_scheme_enum() != 0)
                {
                    need_to_update_color_reservations = true;
                }
                
                if (entity.is_valid())
                    entity.destroy();

                remove_entity_from_registry(entity_name);
            }
        }
        GP_PRINT("Destroyed all entities in layer : ", layer);
    }

    /// @brief Update the color reservations
    /// @details  Use this function to update the color reservations
    /// @note This function is called before the scene is updated so that pick ids are reserved for each entity properly
    void Scene_Manager::update_color_reservations()
    {
        if(need_to_update_color_reservations == false)
        {
            return;
        }
        else
        {
            need_to_update_color_reservations = false;
        }

        std::deque<ecs::Entity>::iterator end = Entity_DataBase.end();

        std::deque<ecs::Entity>::iterator it = Entity_DataBase.begin();

        uint32_t reserved_color_id_end = 10000;

        for (it; it != end; ++it)
        {
            // Temporary Color reservation
            unique_color_reservation colr_reserv;

            // Set its Minima of color id based on previous entity max color id
            colr_reserv._Min_ColorID_ = reserved_color_id_end;

            GeometryDescriptor* MeshComponent = nullptr; 
            
            // get entity's mesh component
            // set color reservation id so that we can know to whom the reservation belongs to 
            if(RenderSystemsManager.has<OpenGL_3_3_RenderDevice>())
            {
               MeshComponent = (it->get<OpenGL_3_3_RenderKernel>().get_descriptor().get());
               colr_reserv._EntityID_ = it->get<OpenGL_3_3_RenderKernel>().get_kernel_id();
            }
            else if(RenderSystemsManager.has<OpenGL_2_1_RenderDevice>())
            {
               MeshComponent = (it->get<OpenGL_2_1_RenderKernel>().get_descriptor().get());
               colr_reserv._EntityID_ = it->get<OpenGL_2_1_RenderKernel>().get_kernel_id();
            }
            else
            {
                GP_ERROR("No RenderKernel and RenderDevice while setting Unique Color Reservation");
                return;
            } 
            
            // Based on previous entitiy mac color id,  let the mesh calculate how many ids it needs
            MeshComponent->set_color_id_reserve_start(reserved_color_id_end);

            // reflect the same in the color reservation component
            colr_reserv._Max_ColorID_ = MeshComponent->get_color_id_reserve_end();

            // After the mesh calculates the required ids by the current mesh set minima for next entity as max of previous entity
            reserved_color_id_end = colr_reserv._Max_ColorID_ + 1;

            unique_colr_reservations[colr_reserv._EntityID_] = colr_reserv;

            GP_TRACE("RESERVED IDS for Entity :", EntityIdxKeyMapRegistry[colr_reserv._EntityID_], " = ", colr_reserv._Min_ColorID_, ", ", colr_reserv._Max_ColorID_);
        }

        last_color_id = reserved_color_id_end - 1;

 
    }

    uint32_t Scene_Manager::get_actual_id(const uint32_t &color_id)
    {
        std::unordered_map<uint32_t, unique_color_reservation>::iterator it = unique_colr_reservations.begin();
        std::unordered_map<uint32_t, unique_color_reservation>::iterator iter_end = unique_colr_reservations.end();

        for (it; it != iter_end; ++it)
        {
            if (color_id >= it->second._Min_ColorID_ && color_id <= it->second._Max_ColorID_)
            {
                return it->second._EntityID_;
            }
        }

        return 0;
    }

    void Scene_Manager::set_system_state(const bool &state)
    {
        m_scene_state_obj.set_render_systems_switch(state);
    }

    bool Scene_Manager::flip_system_state()
    {
        return m_scene_state_obj.flip_render_systems_switch();
    }

    bool Scene_Manager::get_system_state()
    {
        return m_scene_state_obj.get_render_systems_switch();
    }

    SceneState::RenderMode Scene_Manager::get_render_mode() const
    {
        return m_scene_state_obj.get_render_mode();
    }

    SceneState &Scene_Manager::get_scene_state()
    {
        return m_scene_state_obj;
    }

    void Scene_Manager::set_mvp(const glm::mat4 &projection, const glm::mat4 &view, const glm::mat4 &model)
    {
        m_scene_state_obj.m_projection = projection;
        m_scene_state_obj.m_view = view;
        m_scene_state_obj.m_model = model;
    }

    void Scene_Manager::commit_all()
    {
        for (auto entity : RenderableEntitiesManager.with<commit_component>())
        {
            entity.get<commit_component>().commit();
        }
    }

    bool Scene_Manager::commit_entity(const std::string &entity_key)
    {
        if (!has_entity(entity_key))
        {
            GP_TRACE("Entity with key : ", entity_key, " does not exist in the scene");
            return false;
        }
        Entity_Handle entt_handle = get_entity(entity_key);
        entt_handle.GetComponent<commit_component>()->commit();
        return true;
    }

    bool Scene_Manager::uncommit_entity(const std::string &entity_key)
    {
        if (!has_entity(entity_key))
        {
            GP_TRACE("Entity with key : ", entity_key, " does not exist in the scene");
            return false;
        }
        Entity_Handle entt_handle = get_entity(entity_key);
        entt_handle.GetComponent<commit_component>()->uncommit();
        return true;
    }

    bool Scene_Manager::commit_layer(const float &layer)
    {
        bool commit_status = false;
        for (auto entity : RenderableEntitiesManager.with<commit_component>())
        {
            if (entity.get<commit_component>().layer_id() == layer)
            {
                commit_status = true;
                entity.get<commit_component>().commit();
            }
        }
        return commit_status;
    }

    bool Scene_Manager::uncommit_layer(const float &layer)
    {
        bool commit_status = false;
        for (auto entity : RenderableEntitiesManager.with<commit_component>())
        {
            if (entity.get<commit_component>().layer_id() == layer)
            {
                commit_status = true;
                entity.get<commit_component>().uncommit();
            }
        }
        return commit_status;
    }

    void Scene_Manager::clear_screen(const float &r, const float &g, const float &b, const float &a)
    {
        RendererAPI()->glClearColor(r, g, b, a);
        RendererAPI()->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Scene_Manager::reset_scene_registry()
    {
        SceneEntityRegistry.clear();
        EntityIdxKeyMapRegistry.clear();
        unique_colr_reservations.clear();
        Entity_DataBase.clear();
        ecs::EntityManager NewEntityManager;
        RenderableEntitiesManager = std::move(NewEntityManager);
        last_color_id = 0;
        need_to_update_color_reservations = true;

    }
} // namespace GridPro_GFX