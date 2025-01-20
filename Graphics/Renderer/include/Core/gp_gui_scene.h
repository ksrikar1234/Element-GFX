#ifndef GP_GUI_SCENE_H
#define GP_GUI_SCENE_H

#include <unordered_map>
#include <deque>
#include <memory>

#include "ecs.h"

#include "gp_gui_forward_structs.h"

namespace gridpro_gui
{

    namespace Event
    {
        class Publisher;
    }
    
    class GeometryDescriptor;


    /// @brief Scene_Manager class
    /// The scene class is the main
    /// class that holds the entities
    /// and the Render systems that are used to render the entities.
    /// It also holds the color reservations for assigning pick ids to entities 
    /// and the scene state like light, camera, render mode etc.
    class Scene_Manager
    {
    public:
         /// @brief Constructor
         /// @note The Constructor initializes the RenderableEntitiesManager, RenderSystemsManager & PublisherInstance
         /// @note Calls the initialize_render_devices() function (Critical) to check if any render device is available (may be OpenGL, Vulkan, DirectX etc.)
         /// @warning Critical : The Constructor will throw an exception if no render device is available
         Scene_Manager();
        ~Scene_Manager();
         
         bool has_render_device() const;

         bool switch_driver(const unsigned int& driver);
 
    

         // The Only Functions you'll ever need ------------------------+
         /// Creates an entity to the scene
         /// Load a geometry descriptor to the scene
         void commit_geometry(const std::string& in_name, const float& in_layer, const std::shared_ptr<GeometryDescriptor>& geometry_descriptor);

         std::shared_ptr<GeometryDescriptor>& get_geometry(const std::string& in_name);

         /// Updates all Systems
         void update(const float& layer);
         
         /// Updates the mouse event
         void update_mouse_event(const float& x, const float& y);

         /// @brief Set the mvp object generated from Camera
         /// @param projection 
         /// @param view 
         /// @param model 
         void set_mvp(const glm::mat4& projection, const glm::mat4& view, const glm::mat4& model);

         ///------------------------------------------------------------+
         /// @brief Commit to the render device for display
         void commit_all();
         bool commit_entity(const std::string& entity_key);
         bool commit_layer(const float& layer);
         bool uncommit_entity(const std::string& entity_key);
         bool uncommit_layer(const float& layer);

         /// @brief Destroy an entity
         bool destroy_entity(const std::string& entity_key);
         void destroy_entities_in_layer(const float& layer);
         bool remove_entity_from_registry(const std::string& entity_key);

         std::vector<std::pair<std::string, uint32_t>> pick_matrix(const float& center_x, const float& center_y, const float& width, const float& height);
         std::vector<std::pair<std::string, uint32_t>> pick_polygon(const std::vector<float>& polygon_points);

         ///------------------------------------------------------------+
         /// @brief Getters and Setters
         bool has_entity(const std::string& entity_key);


         SceneState::RenderMode get_render_mode() const;
         SceneState& get_scene_state();
         
         void set_system_state(const bool& state);
         bool get_system_state();
         bool flip_system_state();
         
         void clear_screen(const float& r, const float& g, const float& b, const float& a);

    private:
         Entity_Handle get_entity(const std::string& entity_key);
         bool initialize_render_devices();
         void update_color_reservations();
         uint32_t get_actual_id(const uint32_t& color_id);
         void reset_scene_registry();

     private :
     /// @brief Registry of Entities
     /// @note The Below Data Structures are used to Book Keep the Entities and their Color Reservations
     std::deque<ecs::Entity> Entity_DataBase;
     std::unordered_map<std::string, uint32_t> SceneEntityRegistry;
     std::unordered_map<uint32_t, std::string> EntityIdxKeyMapRegistry;
     std::unordered_map<uint32_t, unique_color_reservation> unique_colr_reservations;

     /// @brief ECS Managers
     ecs::EntityManager RenderableEntitiesManager;
     ecs::SystemManager RenderSystemsManager;  

     /// @brief Communication Manager to transport Data like Scene State, Mouse Events, Keyboard Events, Pick Events
     Event::Publisher*  PublisherInstance;

     /// @brief Last Color ID assigned to an Last Entity
     mutable SceneState m_scene_state_obj;
     uint32_t last_color_id;

     /// @warning Critical : Render Device Validation
     private:
     bool has_a_valid_render_device;
     bool need_to_update_color_reservations;
     
    };

} // namespace gridpro_gui


#endif // GP_GUI_SCENE_H