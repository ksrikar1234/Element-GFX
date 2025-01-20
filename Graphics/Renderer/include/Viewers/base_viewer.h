#ifndef _GRIDPRO_BASE_VIEWER_
#define _GRIDPRO_BASE_VIEWER_

#include <memory>
#include <unordered_set>
#include <deque>
#include <vector>
#include <string>
#include <functional>
#include "export.h"

namespace gridpro_gui
{
    class Scene_Manager;
    class OrthographicCamera;
    class GeometryDescriptor;
}

/// ******************** Why AbstractViewerWindow? ****************************

/// The GridPro GUI is a complex application that requires a stable Viewer Window Interface for Rendering
/// The AbtractViewerWindow Provides a Simple Standard Interface that is more than enough for the GridPro Rendering Requirements
/// Think of it as Pen and Paper Way of Drawing
/// Where AbstractViewerWindow is the Paper and the GeometryDescriptor is the Pen 
/// You can Set the Properties of the Paper(Window) and Draw the Geometry by describing it with the Pen(GeometryDescriptor)

/// So a Scientific Programmer can easily use the AbstractViewerWindow Interface to Commit the GeometryDecriptors to the Scene
/// to Render the Geometry without worrying about the OpenGL Rendering and Scene Management

/// ****************************************************************************

/// @class AbstractViewerWindow
/// @brief This Class is intended to Provide a Stable Viewer Window Interface for the GridPro GUI.
/// A GUI Developer can easily inherit from this class and implement the OpenGL Rendering using the OpenGLWidget or any other Windowng System.
/// The Base Viewer Provides a Non Mandatory Protected Default Implementation of One Scene Manager, One Orthographic Camera and Custom OpenGL Drivers.
/// If you want use other Scene Managers like OpenSceneGraph, you can still inherit from this class and use the same Standard Public Interface for Rendering
/// @note This class is a MiddleWare for Rendering the 3D Scene
class LIB_API AbstractViewerWindow
{

///@brief Viewer Pick callback
typedef void (*ViewerPickCallback)(const std::string &entity_name, const uint32_t &sub_entity_id);
typedef void (*ViewerBoxSelectionCallback)(const std::vector<std::pair<std::string, uint32_t>> &entities);
typedef void (*ViewerDoubleClickCallback)(const std::string &entity_name, const uint32_t &sub_entity_id);

public:
    /// @brief  Constructor
    AbstractViewerWindow();

    /// @brief  Destructor
    virtual ~AbstractViewerWindow();

    /// ************** Public API Start *****************
    
    virtual bool accquire_render_context()
    {
        printf("AbstractViewerWindow::accquire_render_context() is not implemented\n");
        return false;
    }

    virtual void update_display()
    {
        printf("AbstractViewerWindow::update_display() is not implemented\n");
    }

    virtual void capture_screen_shot()
    {
        printf("AbstractViewerWindow::capture_screen_shot() is not implemented\n");
    }
    
    /// @brief  This function is used to switch the driver
    void switch_driver(const unsigned int &driver);

    /// @brief  This function is used to commit the geometry to the scene
    /// @param in_name
    /// @param in_geometry
    void commit_geometry(const std::string &in_name, const std::shared_ptr<gridpro_gui::GeometryDescriptor> &in_geometry);

    /// @brief This function is used to commit the geometry to the scene
    /// @param in_name Name of the Entity
    /// @param in_layer_id Layer ID
    /// @param in_geometry Geometry Descriptor
    void commit_geometry(const std::string &in_name, const float &in_layer_id, const std::shared_ptr<gridpro_gui::GeometryDescriptor> &in_geometry);

    /// @brief  This function is used to commit the geometry to the 2D scene
    /// @param in_name
    /// @param in_geometry
    void commit_2d_geometry(const std::string &in_name, const std::shared_ptr<gridpro_gui::GeometryDescriptor> &in_geometry);

    /// @brief This function is used to commit the geometry to the 2D scene
    /// @param in_name Name of the Entity
    /// @param in_layer_id Layer ID
    /// @param in_geometry Geometry Descriptor
    void commit_2d_geometry(const std::string &in_name, const float &in_layer_id, const std::shared_ptr<gridpro_gui::GeometryDescriptor> &in_geometry);

    /// @brief  This function is used to upload all the committed geometries to the scene
    virtual void upload_commits();
    
    /// @brief  This function is used to remove the geometry from the scene
    /// @param in_name Name of the Entity
    virtual void remove_geometry(const std::string &in_name);

    /// @brief  This function is used to hide the geometry from the scene
    void hide_geometry(const std::string &in_name);

    void show_geometry(const std::string &in_name);

    /// @brief This function is used to get back the geometry from the scene
    /// @param in_name Name of the Entity
    /// @return std::shared_ptr<gridpro_gui::GeometryDescriptor>
    std::shared_ptr<gridpro_gui::GeometryDescriptor>& get_geometry(const std::string &in_name);

    /// @brief  This function is used to add a display layer
    /// @param in_layer_id
    void add_display_layer(const float &in_layer_id);

    /// @brief  This function is used to remove(hides) a display layer
    /// @param in_layer_id
    void remove_display_layer(const float &in_layer_id);

    /// @brief  This function is used to clear all the display layers(just hides the entities in the layer, does not destroy them)
    void clear_display_layers();

    /// @brief  This function is used to destroy the entites in a layer
    /// @param in_layer_id
    virtual void destroy_entities_in_layer(const float &in_layer_id);

    /// @brief  This function is used to set the bounding box of the scene & update GL
    /// @param in_bounding_box
    virtual void set_bounding_box(const std::vector<double> &in_bounding_box);

    /// @brief  This function is used to check if blending is enabled
    /// @return  bool
    bool is_blending_enabled() const;

    /// @brief  This function is used to enable or disable blending
    /// @param in_blending_enabled
    void set_blend_state(bool in_blending_enabled);

    /// @brief  This function is used to check if lighting is enabled
    /// @return  bool
    bool is_lighting_enabled() const;

    /// @brief  This function is used to enable or disable lighting
    /// @param in_lighting_enabled
    void set_lighting_state(bool in_lighting_enabled);

    /// @brief  Enable Cluster Selection
    void enable_cluster_selection();

    /// @brief  Disable Cluster Selection and return the selected entities (Similar to OpenGL Selection Mode (GL_SELECT))
    std::unordered_set<std::string>&& stop_cluster_selection();

    /// @brief  This functions clears the scene
    virtual void clear_screen(const float &r, const float &g, const float &b, const float &a);

    /// @brief  This function is used to set the zoom ratio
    virtual void set_zoom_ratio(const float &in_zoom_ratio);

    /// @brief  This function is used to snap the camera to xy plane
    virtual void snap_to_xy_plane();

    /// @brief  This function is used to snap the camera to xz plane
    virtual void snap_to_xz_plane();

    /// @brief  This function is used to snap the camera to yz plane
    virtual void snap_to_yz_plane();

    /// @brief  This function is used to snap the camera to yx plane
    virtual void snap_to_yx_plane();

    /// @brief  This function is used to snap the camera to zx plane
    virtual void snap_to_zx_plane();

    /// @brief  This function is used to snap the camera to zy plane
    virtual void snap_to_zy_plane();

    /// @brief  This function is used to snap the camera to the isometric view
    virtual void snap_to_isometric_view();

    /// @brief  This function is used to zoom in the scene
    virtual void zoom_in();
    
    /// @brief  This function is used to zoom out the scene
    virtual void zoom_out();

    /// @brief  This function is used to frame the scene
    virtual void frame_scene(const char* in_entity_name);
    
    /// @brief  This function is used to set callback for pick event
    void set_pick_callback(ViewerPickCallback in_callback);

    template<typename Instance>
    void set_pick_callback(Instance *instance, void (Instance::*callback)(const std::string &entity_name, const uint32_t &sub_entity_id))
    {
        m_pick_callback = [instance, callback](const std::string &entity_name, const uint32_t &sub_entity_id)
        {
            if(callback)
                (instance->*callback)(entity_name, sub_entity_id);
        };
    }

    /// @brief  This function is used to set box selection callback
    void set_box_selection_callback(ViewerBoxSelectionCallback in_callback)
    {
        m_box_selection_callback = in_callback;
    }

    template<typename Instance>
    void set_box_selection_callback(Instance *instance, void (Instance::*callback)(const std::vector<std::pair<std::string, uint32_t>> &entities))
    {
        m_box_selection_callback = [instance, callback](const std::vector<std::pair<std::string, uint32_t>> &entities)
        {
            if(callback)
                (instance->*callback)(entities);
        };
    }
    
    /// @brief  This function is used to set double click callback
    void set_double_click_callback(ViewerDoubleClickCallback in_callback)
    {
        m_double_click_callback = in_callback;
    }

    template<typename Instance>
    void set_double_click_callback(Instance *instance, void (Instance::*callback)(const std::string &entity_name, const uint32_t &sub_entity_id))
    {
        m_double_click_callback = [instance, callback](const std::string &entity_name, const uint32_t &sub_entity_id)
        {
            if(callback)
                (instance->*callback)(entity_name, sub_entity_id);
        };
    }

    bool holding_a_node() const { return is_holding_a_node; }

    /// ************** Public API End *****************
protected:
    /// Base Viewer Provides a Protected Default Implementation of One Scene Manager, One Orthographic Camera and Custom OpenGL Drivers
    /// The Below function are similar to the OpenGLWidget Member Functions.
    /// See an example Class viewer.h that inherits from this class and OpenGLWidget to know how it works

    /// But if you want to use your own Scene Manager like OpenSceneGraph, you need to reimplement the below functionality
    /// and call the respective functions in your derived Custom OpenGLWidget Class
    void initialize_scene();

    void resize_event(int w, int h);

    void render_update();

    virtual float device_pixel_ratio() const
    {
        return 1.0f;
    }

    void mouse_press_event(const float &x, const float &y, const int& button);
    /// @brief This function is used to update the mouse move event
    /// @return bool (true if needs to update the display)
    bool mouse_move_event(const float &x, const float &y);
    void mouse_release_event(const float& x, const float& y, const int& button);
    void key_press_event(const int& key);
    void wheel_event(const float& delta);

    bool double_click_press_event_handler(float x, float y);
    bool double_click_release_event_handler(float x, float y);

    void handle_zoom_box();
    void handle_polygon_selection();

    void create_and_display_cor();
    void create_and_display_axes();
    void create_and_display_bounding_box();
    void create_and_display_light();
    void create_and_display_2d_background_quad();
    void create_and_display_node_manipulator(const float& x, const float& y, const float& z);
    
    void create_and_display_ray(const float& x, const float& y);

    void set_light_position(const float &in_x, const float &in_y, const float &in_z);

    void pick_call_back(const std::string &entity_name,  const uint32_t &sub_entity_id);

    bool is_double_click(const float &x, const float &y, const bool &is_clicked);

protected :
    std::shared_ptr<gridpro_gui::Scene_Manager>      m_scene;
    std::shared_ptr<gridpro_gui::OrthographicCamera> m_camera;
    std::shared_ptr<gridpro_gui::GeometryDescriptor> m_center_descriptor;
    std::shared_ptr<gridpro_gui::GeometryDescriptor> m_axis_descriptor;
    std::shared_ptr<gridpro_gui::GeometryDescriptor> m_bounding_box_descriptor;
    std::shared_ptr<gridpro_gui::GeometryDescriptor> m_2d_background_quad_descriptor;
    std::shared_ptr<gridpro_gui::GeometryDescriptor> m_light_descriptor;
    std::shared_ptr<gridpro_gui::GeometryDescriptor> m_zoom_box_descriptor;
    std::shared_ptr<gridpro_gui::GeometryDescriptor> m_center_of_rotation_descriptor;
    std::shared_ptr<gridpro_gui::GeometryDescriptor> m_polygon_selection_descriptor;

    std::unordered_set<std::string> m_hidden_geometries;
    std::unordered_set<float> m_current_displayed_layers;
    
    struct geometry_commit
    {
        explicit geometry_commit(const std::string &in_name, const float& in_layer_id, const std::shared_ptr<gridpro_gui::GeometryDescriptor>& in_geometry) 
                 : name(in_name), layer_id(in_layer_id), geometry(in_geometry) {} 
        std::string name;
        float layer_id;
        std::shared_ptr<gridpro_gui::GeometryDescriptor> geometry;
    };

    std::deque<geometry_commit> m_geometry_commit_stack;
    std::deque<geometry_commit> m_2d_geometry_commit_stack;

    bool is_scene_created_successfully = false;
    bool is_view_in_motion = false;
    bool enable_selection_rendering = false;
    bool is_cluster_selection_enabled = false;
    bool is_holding_a_node = false;
    bool is_drawing_zoom_box = false;
    bool is_drawing_polygon_selection = false;

    struct currently_holded_node
    {
        std::string entity_name;
        uint32_t node_index;
    };

    currently_holded_node m_currently_holded_node;

    std::string previouly_hovered_entity_name;

    std::string previouly_hovered_sub_entity_name;
    
    uint32_t previously_hovered_sub_entity_id;

    std::unordered_set<std::string> m_selected_entities;

    std::vector<std::pair<std::string, uint32_t>> m_box_selected_entities;

    std::string m_hovered_sub_entity_name;

    std::function<void(const std::string &entity_name, const uint32_t &sub_entity_id)> m_pick_callback;
    std::function<void(const std::vector<std::pair<std::string, uint32_t>> &entities)> m_box_selection_callback;
    std::function<void(const std::vector<std::pair<std::string, uint32_t>> &entities)> m_polygon_selection_callback;
    std::function<void(const std::string &entity_name, const uint32_t &sub_entity_id)> m_double_click_callback;


    struct LightPosition
    {
        LightPosition() = default;
        LightPosition(const float &in_x, const float &in_y, const float &in_z) : x(in_x), y(in_y), z(in_z) {}
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };
    
    LightPosition m_light_position;

    // 2D Scene Projection Matrix
    float m_2d_projection_matrix[16];

    // Screen Properties
    struct ScreenDimensions
    {
        float width = 0.0f;
        float height = 0.0f;
        float aspect_ratio() const
        {
            return width / height;
        }
    };

    ScreenDimensions m_screen_dims;

    double DevicePixelRatio = 1.0;

    // Peripherals
    float m_MouseSensitivity = 0.01f;
    
    enum MouseButton : int
    {
        None = 0,
        Left = 1,
        Right = 2,
        Middle = 3
    };
    
    struct MouseState
    {
        MouseState() = default;
        MouseState(const float &in_x, const float &in_y, const MouseButton &in_button, const bool &in_is_pressed) : x(in_x), y(in_y), button(in_button), is_pressed(in_is_pressed) {}
        MouseState(const MouseState &in_state) : x(in_state.x), y(in_state.y), button(in_state.button), is_pressed(in_state.is_pressed) {}
        
        MouseState& operator=(const MouseState &in_state)
        {
            x = in_state.x;
            y = in_state.y;
            button = in_state.button;
            is_pressed = in_state.is_pressed;
            return *this;
        }

        bool operator==(const MouseState &in_state) const
        {
            return x == in_state.x && y == in_state.y && button == in_state.button && is_pressed == in_state.is_pressed;
        }

        float x = 0.0f;
        float y = 0.0f;
        MouseButton button = MouseButton::None;
        bool is_pressed = false;
    };

    MouseState m_current_mouse_state;
    MouseState m_last_mouse_press_state;
    MouseState m_last_mouse_release_state;     

    bool m_ctrl_key = false;
    bool m_shift_key = false;
    bool m_alt_key = false;
    bool m_space_key = false;
};

#endif // _GRIDPRO_BASE_VIEWER_