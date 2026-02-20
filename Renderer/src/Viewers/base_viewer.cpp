#include "base_viewer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gp_gui_scene.h"
#include "gp_gui_camera.h"

#include "gp_gui_geometry_descriptor.h"
#include "gp_gui_entity_handle.h"

#include "gp_gui_communications.h"
#include "gp_gui_events.h"
#include "gp_gui_debug.h"

#include <chrono>
#include <cmath>

#include <set>
#include <future>

// For Reading the workspace Config
#include <nlohmann/json.hpp>
#include <fstream>

#include "Primitives_3D/gp_sphere.hpp"
#include "gp_gizmo.hpp"


using namespace GridPro_GFX;

AbstractViewerWindow::AbstractViewerWindow()
{
    enable_selection_rendering = true;    
    enable_selection_box = true;
    m_current_displayed_layers.insert(GL_LAYER_1);
    m_workplane =  WorkPlane(1.0, 0.0, 0.0, 0.0);
    m_workplane.size = 5.0f;
    is_workplane_active = false;
    previouly_hovered_entity_name = "NULL_ENTITY";
    is_view_changed = true;
    is_cluster_selection_enabled = false;
    DevicePixelRatio = 1.0f;
    is_holding_a_node = false;
    is_drawing_zoom_box = false;
    is_drawing_polygon_selection = false;
}

AbstractViewerWindow::~AbstractViewerWindow()
{
    
}

void AbstractViewerWindow::initialize_scene()
{
    GP_COLOR_PRINT(GP_COLOR::BRIGHT_GREEN, "Initiaising Scene");
    accquire_render_context();
    is_scene_created_successfully = true;

    m_scene = std::make_shared<Scene_Manager>();

    is_scene_created_successfully = m_scene->has_render_device();

    if (is_scene_created_successfully == false)
    {
        GP_ERROR("Init GL Scene not created successfully\n");
    }

    m_camera = std::make_shared<OrthographicCamera>();
    m_camera->set_updated_bounding_box({glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f)});

    m_axis_descriptor         = std::make_shared<GeometryDescriptor>();
    m_light_descriptor        = std::make_shared<GeometryDescriptor>();
    m_center_descriptor       = std::make_shared<GeometryDescriptor>();
    m_bounding_box_descriptor = std::make_shared<GeometryDescriptor>();
    m_zoom_box_descriptor     = std::make_shared<GeometryDescriptor>();
    m_axis_end_point_descriptor = std::make_shared<GeometryDescriptor>();

    m_polygon_selection_descriptor  = std::make_shared<GeometryDescriptor>();

    m_center_of_rotation_descriptor = std::make_shared<GeometryDescriptor>();

    m_2d_background_quad_descriptor = std::make_shared<GeometryDescriptor>();

    m_workplane_descriptor =  std::make_shared<GeometryDescriptor>();
    
    create_and_display_cor();
    create_and_display_axes();
    create_and_display_bounding_box();
    create_and_display_2d_background_quad();
    create_and_display_gizmo();

    m_MouseSensitivity = 0.02f;
    
    set_lighting_state(true);
    m_scene->get_scene_state().enable_blending = true;
}

void AbstractViewerWindow::switch_driver(const GLenum &driver)
{
    accquire_render_context();
    m_scene->switch_driver(driver);
}

void AbstractViewerWindow::resize_event(int w, int h)
{
    accquire_render_context();
    m_camera->update_projection();
    DevicePixelRatio = device_pixel_ratio();
    m_screen_dims.width = w*DevicePixelRatio;
    m_screen_dims.height = h * DevicePixelRatio;
    m_camera->set_screen_dims(glm::vec2(m_screen_dims.width, m_screen_dims.height));
    is_view_changed = true;
    enable_selection_rendering = true;
    m_scene->get_scene_state().set_screen_dims(m_screen_dims.width, m_screen_dims.height);
}

void AbstractViewerWindow::render_update()
{
    m_scene->set_mvp(m_camera->GetProjectionMatrix(), m_camera->GetViewMatrix(), glm::mat4(1.0f));

    glm::vec3 light_pos = m_camera->get_light_position();

    set_light_position(light_pos.x, light_pos.y, light_pos.z);
    
    create_and_display_light();
    create_and_display_workplane();
    create_and_display_mouse_ray();
    
    upload_commits();

    m_scene->commit_all();

    for (auto& hidden_geometry : m_hidden_geometries)
    {
        m_scene->uncommit_entity(hidden_geometry);
    }

    if (enable_selection_rendering == true)
    {
        m_scene->update((GL_LAYER_PICKABLE));
    }
    
    if(gp_std::is_debug_flag_set("GP_SELECTION_DEBUG"))
    return;

    clear_screen(0.117647, 0.117647, 0.117647, 1.0f);

    // 2D Canvas Background Rendering
    m_scene->update(GL_LAYER_BACKGROUND_2D);

    for (auto& active_layer_id : m_current_displayed_layers)
    {
        m_scene->update(active_layer_id);
    }

    // 2D Scene Rendering
    m_scene->update(GL_LAYER_FOREGROUND_2D);
}

void AbstractViewerWindow::mouse_press_event(const float &x, const float &y, const int &button)
{
    // **********************************************************
    // Handling the mouse press event
    // **********************************************************
    m_prev_mouse_state = {x, y, static_cast<MouseButton>(button), true};
    m_last_mouse_press_state = m_prev_mouse_state;
    
    // **********************************************************
    // Draw the frame with Selection Mode so that the Selection Buffer is filled
    // Selection Mode rendering is only done when view is not in motion
    // So first we set the view in motion to false, draw the frame and then set it back to true
    // **********************************************************
    enable_selection_rendering = true;
    update_display();
    enable_selection_rendering = false;
    // *************************END******************************

    m_scene->update_mouse_event(x * DevicePixelRatio, y * DevicePixelRatio);

    Event::Subscription sub("mouse_press");

    bool is_double_clicked = is_double_click(x * DevicePixelRatio, y * DevicePixelRatio, true);

    if (is_double_clicked)
    {
        if (sub.getPickEvent().getEntityKey() != "NULL_ENTITY")
        {
            double_click_state = true;
            double_click_x = x;
            double_click_y = y;
            double_clicked_entity = sub.getPickEvent().getEntityKey();
            double_clicked_sub_entity = sub.getPickEvent().getSubEntityID();
            double_click_press_event_handler(x * DevicePixelRatio, y * DevicePixelRatio);
            return;
        }
    }

    if(m_prev_mouse_state.button == MouseButton::Right && enable_selection_box)
    {
       is_drawing_zoom_box = true;
       handle_zoom_box();
       return;
    }
    else
    {
        is_drawing_zoom_box = false;
    }
    
    if(m_prev_mouse_state.button == MouseButton::Left)
    {
        if(is_drawing_polygon_selection)
        {
           handle_polygon_selection();
           update_display();
           return;
        }
    }

 
    m_pick_callback(sub.getPickEvent().getEntityKey(), sub.getPickEvent().getSubEntityID());
    

    if (sub.getPickEvent().getEntityKey() != "NULL_ENTITY")
    {
        // Cluster Selection is different from normal selection
        // Here we select the entities one by one
        // Each entity can be selected or deselected by clicking on it 
        if (is_cluster_selection_enabled)
        {
           handle_cluster_selection();
        }
    }
}

bool AbstractViewerWindow::mouse_move_event(const float &x, const float &y)
{   
    bool need_redraw = false;
        
    if(is_drawing_polygon_selection)
    {
        m_prev_mouse_state.x = x; m_prev_mouse_state.y = y;
        handle_polygon_selection();
        update_display();
        return true;
    }

    if(is_drawing_zoom_box)
    {
       handle_zoom_box();
       update_display();
       m_prev_mouse_state.x = x; m_prev_mouse_state.y = y;
       return true;
    }

    if(is_view_changed)
    {
       // Usually we dont want to render in selection mode when the view is in motion
       // But exceptional case is when we are resizing or adding new entity we need to update the selection frame buffer and turn it off back
       update_display();
       enable_selection_rendering = false;
    }  
    
    m_scene->update_mouse_event(x * DevicePixelRatio, y * DevicePixelRatio);

    Event::Subscription sub("mouse_press");

    // **********************************************************
    // Highlighting Mechanism for the entity that is hovered
    // If the entity is hovered, it will be highlighted
    // If the entity is not hovered, it will be unhighlighted
    // **********************************************************

    //********** Sub Entity Highlighting Mechanism ********************
    need_redraw = need_redraw || handle_subentity_highlighting();
    // ****************************************************************  

    // ******* Geometry Hover Highlighting & Node Manipulation Mechanism ****************************
    if(handle_geometry_highlighting_and_node_manipulation(x, y, need_redraw))
    {
        // This means if the node_manipulator is being used we dont need to 
        // process the mouse_move_event anymore further
        return need_redraw;
    }
    //*****************************************************************

    // **********************************************************
    // Camera Movement Mechanism
    // **********************************************************
    float xOffset = x - m_prev_mouse_state.x;
    float yOffset = m_prev_mouse_state.y - y; // Reversed since y-coordinates go from bottom to top
    
    if (m_prev_mouse_state.button == MouseButton::Left)
    {
        m_camera->processMousePan(-xOffset, yOffset);
        enable_selection_rendering = false;
        create_and_display_cor();
        update_display();
        need_redraw = true;
        is_view_changed = true;
    }
    
    else if (m_prev_mouse_state.button == MouseButton::Middle)
    {
        m_camera->processMouseRotation(-xOffset, yOffset);
        // m_camera->processMousePanForCOR(0, 0);
        enable_selection_rendering = false;
        create_and_display_cor();
        update_display();
        is_view_changed = true;
        need_redraw = true;
    }
    else
    {
        is_view_changed = false;
    }

    // *************************END******************************
    
    m_prev_mouse_state.x = x;
    m_prev_mouse_state.y = y;

    return need_redraw;
}

void AbstractViewerWindow::mouse_release_event(const float &x, const float &y, const int &button)
{
    m_last_mouse_release_state = MouseState(x, y, static_cast<MouseButton>(button), false);
    m_prev_mouse_state = MouseState(x, y, MouseButton::None, false);
    // **********************************************************
    enable_selection_rendering = true;
    is_holding_a_node = false;
    is_drawing_zoom_box = false;
    
    update_display();
    if(double_click_release_event_handler(x * DevicePixelRatio, y * DevicePixelRatio))
    {
        return;
    }

    handle_zoom_box();
    update_display();
    is_view_changed = false;
}

/// @brief This function is used to handle the mouse wheel event
/// @param delta_y
void AbstractViewerWindow::wheel_event(const float &delta_y)
{
    m_camera->processMouseScroll(delta_y);
    m_camera->processMousePan(0, 0);
    enable_selection_rendering = false;
    create_and_display_cor();
    update_display(); // Request redraw
}


/// @brief This function is used to handle the keyboard press event
/// @param key
void AbstractViewerWindow::key_press_event(const int32_t& key)
{
    // For Mac OS (Do not switch driver)
    #ifdef  __APPLE__
    return;
    #endif
    // if(key == 0)
    // {
    //     switch_driver(GL_DRIVER_OPENGL_2_1);
    // }
    // else if(key == 1)
    // {
    //     switch_driver(GL_DRIVER_OPENGL_3_3);
    // }
    if(key == 1)
    {
        is_drawing_polygon_selection = (!is_drawing_polygon_selection); 
        if(is_drawing_polygon_selection)
        {
            m_polygon_selection_descriptor->copy_pos_array({});
            GP_PRINT("Polygon Selection Mode Activated"); 
        }
        else
        {
            GP_PRINT("Polygon Selection Mode Deactivated");
            handle_polygon_selection();
        }
    }
}

bool AbstractViewerWindow::handle_subentity_highlighting()
{
    bool need_redraw = false;
    Event::Subscription sub("mouse_press");

    // **********************************************************
    // Highlighting Mechanism for the entity that is hovered
    // If the entity is hovered, it will be highlighted
    // If the entity is not hovered, it will be unhighlighted
    // **********************************************************

    //*****  Sub Entity Highlighting Mechanism  ***************

    hide_geometry("HOVERED_SUB_ENTITY");  

    if (sub.getPickEvent().getEntityKey() != "NULL_ENTITY" && !is_view_changed) // Cant move the world and pick at the same time
    {
        const std::string selected_entity_key = sub.getPickEvent().getEntityKey();
        uint32_t sub_entity_id = sub.getPickEvent().getSubEntityID();
        auto& hovered_descriptor = get_geometry(selected_entity_key);
        
        if(!(*hovered_descriptor)->isHighlightable()) return need_redraw;

        if((*hovered_descriptor)->get_pick_scheme_enum() == GL_PICK_BY_VERTEX)
        {
        float* vertex = (*hovered_descriptor)->get_vertex_ref(sub_entity_id);
        std::vector<float> pos_array;
        for(int i = 0; i < 3 ; ++i)
        {
            pos_array.push_back(vertex[i]);
        } 

        auto sub_entity_descriptor = std::make_shared<GeometryDescriptor>();
        sub_entity_descriptor->set_current_primitive_set("Hovered Entity", GL_POINTS);
        sub_entity_descriptor->move_pos_array(std::move(pos_array));
        sub_entity_descriptor->set_fill_color(255,0,0,255);
        sub_entity_descriptor->set_point_size(20.0f);
        commit_geometry("HOVERED_SUB_ENTITY", sub_entity_descriptor); 
        show_geometry("HOVERED_SUB_ENTITY");
        need_redraw = true;
        }
    
        else if((*hovered_descriptor)->get_pick_scheme_enum() == GL_PICK_BY_PRIMITIVE)
        {
          if((*hovered_descriptor)->get_primitive_type_enum() == GL_LINES)
          {
            const std::string selected_entity_key = sub.getPickEvent().getEntityKey();
            uint32_t sub_entity_id = sub.getPickEvent().getSubEntityID();
            auto &hovered_descriptor = get_geometry(selected_entity_key);
            std::vector<float> pos_array = (*hovered_descriptor)->get_picked_primitive(sub_entity_id);
            auto sub_entity_descriptor = std::make_shared<GeometryDescriptor>();
            sub_entity_descriptor->set_current_primitive_set("Hovered Entity", (*hovered_descriptor)->get_primitive_type_enum());
            sub_entity_descriptor->move_pos_array(std::move(pos_array));
            sub_entity_descriptor->set_fill_color(255,0,0,255);
            float curr_lw = (*sub_entity_descriptor)->get_line_width();
            sub_entity_descriptor->set_line_width(curr_lw + 20.0f);
            commit_geometry("HOVERED_SUB_ENTITY", sub_entity_descriptor); 
            show_geometry("HOVERED_SUB_ENTITY");
            need_redraw = true;
           }
        }
    }    
    else
    {
        hide_geometry("HOVERED_SUB_ENTITY");
    }

    if (is_holding_a_node)
    {
        const std::string selected_entity_key = m_currently_holded_node.entity_name;
        uint32_t sub_entity_id = m_currently_holded_node.node_index;
        auto &hovered_descriptor = get_geometry(selected_entity_key);

        if((*hovered_descriptor)->get_pick_scheme_enum() == GL_PICK_BY_PRIMITIVE || (*hovered_descriptor)->get_pick_scheme_enum() == GL_PICK_BY_VERTEX)
        {
        float* vertex = (*hovered_descriptor)->get_vertex_ref(sub_entity_id);
        std::vector<float> pos_array;
        for(int i = 0; i < 3 ; ++i)
        {
            pos_array.push_back(vertex[i]);
        } 

        auto sub_entity_descriptor = std::make_shared<GeometryDescriptor>();
        sub_entity_descriptor->set_current_primitive_set("Hovered Entity", GL_POINTS);
        sub_entity_descriptor->move_pos_array(std::move(pos_array));
        sub_entity_descriptor->set_fill_color(255,0,0,255);
        sub_entity_descriptor->set_point_size(20.0f);
        commit_geometry("HOVERED_SUB_ENTITY", sub_entity_descriptor);  
        show_geometry("HOVERED_SUB_ENTITY");
        need_redraw = true;
        }
    }

    return need_redraw;
}

bool AbstractViewerWindow::handle_geometry_highlighting_and_node_manipulation(const float& x, const float& y, bool& need_redraw)
{
    bool need_to_return_early = false;
    Event::Subscription sub("mouse_press");

    // *******************************************************************
    // ******* Node Manipulation Mechanism + Geometry Hover Highlighting Mechanism ********
    if(is_holding_a_node) // If a node is being held already
    {
        auto& curr_entity_descriptor = get_geometry(m_currently_holded_node.entity_name);
        glm::vec3 translation_vector = m_camera->get_world_space_translation_vector(glm::vec2(m_prev_mouse_state.x, m_prev_mouse_state.y), glm::vec2(x, y));
        curr_entity_descriptor->translate_vertex({ translation_vector.x, translation_vector.y, translation_vector.z } , m_currently_holded_node.node_index);
        m_prev_mouse_state.x = x; m_prev_mouse_state.y = y;
        float* node_pos = (*curr_entity_descriptor)->get_vertex_ref(m_currently_holded_node.node_index);

        if(is_workplane_active && m_workplane.is_valid())
         {
            Point point_on_plane = m_workplane.project_onto_plane(node_pos[0], node_pos[1], node_pos[2]);
            node_pos[0] = point_on_plane.x;
            node_pos[1] = point_on_plane.y;
            node_pos[2] = point_on_plane.z; 
        }   

        enable_selection_rendering = false;
        update_display();
        need_redraw = true;
        need_to_return_early = true;
        return need_to_return_early;
    }

    if (sub.getPickEvent().getEntityKey() != "NULL_ENTITY")
    {
        const std::string selected_entity_key = sub.getPickEvent().getEntityKey();
        uint32_t sub_entity_id = sub.getPickEvent().getSubEntityID();
        
        auto& curr_entity_descriptor = get_geometry(selected_entity_key);

        // if user tries to hold and move a node on a geometry that has node manipulation enabled
        if((curr_entity_descriptor->isNodeManipulationEnabled() && m_prev_mouse_state.button == MouseButton::Left))
        {
            m_currently_holded_node.entity_name = selected_entity_key;
            m_currently_holded_node.node_index  = sub_entity_id;
            glm::vec3 translation_vector = m_camera->get_world_space_translation_vector(glm::vec2(m_prev_mouse_state.x, m_prev_mouse_state.y), glm::vec2(x, y)) * 1.25f;
            curr_entity_descriptor->translate_vertex({ translation_vector.x, translation_vector.y, translation_vector.z }, sub_entity_id);
            m_prev_mouse_state.x = x; m_prev_mouse_state.y = y;
            is_holding_a_node = true;
            float* node_pos = (*curr_entity_descriptor)->get_vertex_ref(m_currently_holded_node.node_index);

            if(is_workplane_active && m_workplane.is_valid())
            {
               Point point_on_plane = m_workplane.project_onto_plane(node_pos[0], node_pos[1], node_pos[2]);
               node_pos[0] = point_on_plane.x;
               node_pos[1] = point_on_plane.y;
               node_pos[2] = point_on_plane.z; 
            }
            
            enable_selection_rendering = false;
            update_display();
            need_redraw = true;
            need_to_return_early = true;
            return need_to_return_early;
        }

        if(!is_view_changed)
        {
        if (previouly_hovered_entity_name != "NULL_ENTITY")
        {
            auto& previously_hovered_entity_descriptor = get_geometry(previouly_hovered_entity_name);
            previously_hovered_entity_descriptor->set_hover_highlights(false);
        }

        auto& currently_hovered_entity_descriptor = get_geometry(selected_entity_key);

        currently_hovered_entity_descriptor->set_hover_highlights(true);

        if (previouly_hovered_entity_name != selected_entity_key)
        {
            enable_selection_rendering = false;
            update_display();
            need_redraw = true;
            previouly_hovered_entity_name = selected_entity_key;
        }
        }
    }
    else
    {   
        if (previouly_hovered_entity_name != "NULL_ENTITY")
        {
            auto& currently_hovered_entity_descriptor = get_geometry(previouly_hovered_entity_name);
            currently_hovered_entity_descriptor->set_hover_highlights(false);
            previouly_hovered_entity_name = "NULL_ENTITY";
            enable_selection_rendering = false;
            update_display();
            need_redraw = true;
        }
    }
    
    return need_to_return_early;
    // *************************END******************************
}

void AbstractViewerWindow::handle_polygon_selection()
{
    if(is_drawing_polygon_selection == false || is_polygon_selection_ready == true)
    {
        hide_geometry("polygon_selection");

        if (is_polygon_selection_ready)
        {
            enable_selection_rendering = true;
            update_display();
            enable_selection_rendering = false;

            auto& polygon_vertices = (*m_polygon_selection_descriptor)->positions_vector();

            std::vector<float> polygon_coords;
            
            for(int i = 0; i < polygon_vertices.size(); i+=3)
            {
                polygon_coords.push_back(polygon_vertices[i]);
                polygon_coords.push_back(polygon_vertices[i+1]);
            }
            
            auto denormalize_coords = [this](float& x,  float& y) 
            {
                float x_denorm = (x + 1.0f) * m_screen_dims.width / 2.0f;
                float y_denorm = (1.0f - y) * m_screen_dims.height / 2.0f;
                x = x_denorm; y = y_denorm;
            };

            for(int i = 0; i < polygon_coords.size(); i+=2)
            {
                denormalize_coords(polygon_coords[i], polygon_coords[i+1]);
            }

            std::vector<std::pair<std::string, uint32_t>> selected_entities = m_scene->pick_polygon(polygon_coords);
            
            std::cout << "Selected Entities List : " << std::endl;

            for (auto &entity : selected_entities)
            {
                std::cout << entity.first << " " << entity.second << std::endl;
            }

            is_drawing_polygon_selection = false;
            is_polygon_selection_ready = false;
        }
        
        update_display();
        return;
    }

    show_geometry("polygon_selection");

    auto eval_normalised_coords = [this](const float &mouse_x, const float &mouse_y) -> std::pair<float , float> 
    {
        float x = mouse_x * DevicePixelRatio;
        float y = mouse_y * DevicePixelRatio;
        float x_norm = (2.0f * x / m_screen_dims.width) - 1.0f;
        float y_norm = 1.0f - (2.0f * y / m_screen_dims.height);
        return std::pair<float , float>(x_norm, y_norm);
    };
    
    float x, y;
    auto coords = eval_normalised_coords(m_prev_mouse_state.x, m_prev_mouse_state.y); 
    x = coords.first; y = coords.second;

    m_polygon_selection_descriptor->set_current_primitive_set("polygon_selection", GL_LINE_STRIP);
    
    // This Means that the user has clicked and we extending the polygon
    if(m_prev_mouse_state == m_last_mouse_press_state)
    {
        if((*m_polygon_selection_descriptor)->positions_vector().size() > 9)
        {
            // if the current point is near the first point, then close the polygon 
            const float* p0 = &(*m_polygon_selection_descriptor)->positions_vector()[0];
            
            if(std::abs(p0[0] - x) < 0.05f && std::abs(p0[1] - y) < 0.05f)
            {
                m_polygon_selection_descriptor->set_fill_color(255, 0, 0, 255);
                m_polygon_selection_descriptor->set_line_width(2.0f);
                uint32_t vertex_id = (*m_polygon_selection_descriptor)->positions_vector().size()/3;                
                m_polygon_selection_descriptor->update_vertex( { p0[0], p0[1], 0.0f }, vertex_id - 1);
                m_geometry_commit_stack.emplace_front(geometry_commit("polygon_selection", GL_LAYER_FOREGROUND_2D, m_polygon_selection_descriptor));
                is_polygon_selection_ready = true;
                enable_selection_rendering = true;
                update_display();
                enable_selection_rendering = false;
                return;
            }
        }
        // else we are just adding a new point to the polygon
        // First point is added
        m_polygon_selection_descriptor->push_pos3f(x, y, 0.0f);

    }
    else
    {
        // Only last point is updated (Line is flexible)
        uint32_t vertex_id = (*m_polygon_selection_descriptor)->positions_vector().size()/3;
        m_polygon_selection_descriptor->update_vertex( { x, y, 0.0f }, vertex_id - 1);
    }
    
    m_polygon_selection_descriptor->set_fill_color(255, 255, 255, 255);
    m_polygon_selection_descriptor->set_line_width(2.0f);
    m_geometry_commit_stack.emplace_front(geometry_commit("polygon_selection", GL_LAYER_FOREGROUND_2D, m_polygon_selection_descriptor));
}

void AbstractViewerWindow::handle_cluster_selection()
{
    // Cluster Selection is different from normal selection
    // Here we select the entities one by one
    // Each entity can be selected or deselected by clicking on it as shown in logic
    Event::Subscription sub("mouse_press");
    const std::string selected_entity_key = sub.getPickEvent().getEntityKey();
    uint32_t sub_entity_id = sub.getPickEvent().getSubEntityID();
    
    auto& selected_entity_descriptor = get_geometry(selected_entity_key);

    // If the entity is already selected, then deselect it
    if (m_selected_entities.find(selected_entity_key) != m_selected_entities.end())
    {
        m_selected_entities.erase(selected_entity_key);
        selected_entity_descriptor->set_selection_highlights(false);
    }
    // Else Highlight & select the entity
    else
    {
        selected_entity_descriptor->set_selection_highlights(true);
        m_selected_entities.insert(selected_entity_key);
    }
}

void AbstractViewerWindow::handle_zoom_box()
{ 
    if(is_drawing_zoom_box == false)
    {
        hide_geometry("zoom_box");
        
        float center_x = (m_last_mouse_press_state.x * DevicePixelRatio + m_prev_mouse_state.x * DevicePixelRatio) / 2.0f;
        float center_y = (m_last_mouse_press_state.y * DevicePixelRatio + m_prev_mouse_state.y * DevicePixelRatio) / 2.0f;

        float width  = fabs(m_last_mouse_press_state.x * DevicePixelRatio - m_prev_mouse_state.x * DevicePixelRatio);
        float height = fabs(m_last_mouse_press_state.y * DevicePixelRatio - m_prev_mouse_state.y * DevicePixelRatio);
        
        m_box_selected_entities.clear();
        m_box_selected_entities = m_scene->pick_matrix(center_x, center_y, width, height);
        std::set<std::pair<std::string, uint32_t>> temp_selected_entities(m_box_selected_entities.begin(), m_box_selected_entities.end());

        m_box_selected_entities.resize(temp_selected_entities.size());
 
        int i = 0;

        for(auto& entity : temp_selected_entities)
        {
            m_box_selected_entities[i] = entity;
            ++i;
        }
        
        update_display();
        
        for(auto& n : m_box_selected_entities)
        {
            std::cout << "Selected Entity : " << n.first << ", SubEntity: " << n.second << "\n"; 
        }

        m_box_selection_callback(m_box_selected_entities);
        
        return;
    }

    show_geometry("zoom_box");

    // Normalise the mouse coordinates
    float x1 = m_prev_mouse_state.x * DevicePixelRatio;
    float y1 = m_prev_mouse_state.y * DevicePixelRatio;
    float x2 = m_last_mouse_press_state.x * DevicePixelRatio;
    float y2 = m_last_mouse_press_state.y * DevicePixelRatio;

    float x_min = (x1 < x2) ? x1 : x2;
    float x_max = (x1 > x2) ? x1 : x2;
    float y_min = (y1 < y2) ? y1 : y2;
    float y_max = (y1 > y2) ? y1 : y2;


    // Convert the screen coordinates to normalise between -1 and 1
    float x_min_norm = (2.0f * x_min / m_screen_dims.width) - 1.0f;
    float x_max_norm = (2.0f * x_max / m_screen_dims.width) - 1.0f;
    float y_min_norm = 1.0f - (2.0f * y_min / m_screen_dims.height);
    float y_max_norm = 1.0f - (2.0f * y_max / m_screen_dims.height);

    std::vector<float> zoom_box_coords =
    {
        x_min_norm, y_min_norm, 0.0f, // Bottom Left
        x_max_norm, y_min_norm, 0.0f, // Bottom Right
        x_max_norm, y_max_norm, 0.0f, // Top Right
        x_min_norm, y_max_norm, 0.0f  // Top Left
    };

    m_zoom_box_descriptor->set_current_primitive_set("zoom_box", GL_LINE_LOOP);
    m_zoom_box_descriptor->move_pos_array(std::move(zoom_box_coords));
    m_zoom_box_descriptor->set_fill_color(255, 255, 255, 255);
    m_zoom_box_descriptor->set_line_width(2.0f);
    m_geometry_commit_stack.emplace_front(geometry_commit("zoom_box", GL_LAYER_FOREGROUND_2D, m_zoom_box_descriptor));
}

// This Class is only used in this source file so it is defined here
struct DoubleClickDetector
{
    // Singleton instance method
    static DoubleClickDetector* instance()
    {
        static DoubleClickDetector instance;
        return &instance;
    }

    // Method to update the state of the detector based on new click
    static bool update(const float &in_x, const float &in_y, const bool & is_clicked)
    {
        auto* inst = instance();

        if (is_clicked)
        {
            // Get current time
            auto current_time = std::chrono::steady_clock::now();

            // Check if this is within the double click time threshold
            if (inst->click_count > 0 && inst->within_threshold(current_time) && inst->near_last_click(in_x, in_y))
            {
                inst->is_double_click = true;
                inst->click_count = 0;  // Reset count after double click detected
            }
            else
            {
                // Reset for the next possible click
                inst->is_double_click = false;
                inst->last_click_time = current_time;
                inst->x = in_x;
                inst->y = in_y;
                inst->click_count = 1;  // Start counting clicks
            }
        }

        return inst->is_double_click;
    }

    private:
    // Private constructor for singleton
    DoubleClickDetector() = default;

    // Utility to check time threshold for double click
    bool within_threshold(const std::chrono::steady_clock::time_point& current_time) const
    {
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_click_time).count();
        return elapsed_time <= double_click_time_threshold;
    }

    // Utility to check if the new click is near the previous click coordinates
    bool near_last_click(const float &in_x, const float &in_y) const
    {
        const float click_tolerance = 5.0f; // Tolerance in pixels
        return (fabs(in_x - x) < click_tolerance) && (fabs(in_y - y) < click_tolerance);
    }

    // Variables to track state
    float x = 0.0f;
    float y = 0.0f;
    bool is_double_click = false;
    int click_count = 0;
    std::chrono::steady_clock::time_point last_click_time;

    // Double click detection threshold (in milliseconds)
    const int double_click_time_threshold = 200;  // Half a second for double-click
};


bool AbstractViewerWindow::is_double_click(const float &x, const float &y, const bool &is_clicked)
{
    return DoubleClickDetector::update(x, y, is_clicked);
}

bool AbstractViewerWindow::double_click_press_event_handler(float x, float y)
{
    return false;

    printf("Double clicked on :", double_clicked_entity.c_str());
    printf("Double clicked on Sub entity id :", double_clicked_sub_entity);

    if (double_clicked_entity != "NULL_ENTITY" || double_clicked_entity.find("double_clicked_entity") == std::string::npos)
    {
        auto &entity = get_geometry(double_clicked_entity);
        auto &double_clicked_entity_desc = entity;
        double_clicked_entity_desc->swap_pick_scheme(GL_PICK_BY_PRIMITIVE);
        commit_geometry(double_clicked_entity, double_clicked_entity_desc);
        enable_selection_rendering = true;
        update_display();
        enable_selection_rendering = false;
    }
    return true;
}

bool AbstractViewerWindow::double_click_release_event_handler(float x, float y)
{
    if (double_click_state)
    {
        enable_selection_rendering = true;
        update_display();
        enable_selection_rendering = false;
        
        m_scene->update_mouse_event(x, y);
        Event::Subscription sub_final("mouse_press");
        std::string FinalSelectedEntity = sub_final.getPickEvent().getEntityKey();
        
        // Evaluate the COR using mouse_x, mouse_y, z buffer depth at that point  
        float curr_depth = sub_final.getPickEvent().getDepth();
        glm::vec3 cor = m_camera->screen_to_world(glm::vec2(x, y), curr_depth);
        m_camera->set_center_of_rotation(cor.x, cor.y, cor.z);
        
        double_click_state = false;
        create_and_display_cor();
        return true;
    }

    return false;

}

void AbstractViewerWindow::commit_geometry(const std::string &in_name, const std::shared_ptr<GeometryDescriptor> &geometry_descriptor)
{
    commit_geometry(in_name, GL_LAYER_1, geometry_descriptor);
}

void AbstractViewerWindow::commit_geometry(const std::string &in_name, const float &in_layer, const std::shared_ptr<GeometryDescriptor> &geometry_descriptor)
{
    /* The Below Line 
       
       is_view_changed = true;
       enable_selection_rendering = true;

       is added because we need to update the selection Buffer with new geometries 

       This might not be correct name but we need to add is_view_updated but it would be of same effect.
       check line 268 in mouse_move_event
    */
    is_view_changed = true;
    enable_selection_rendering = true;
    /*--------------------------------------------------------------------------------------------*/
    m_geometry_commit_stack.emplace_back(geometry_commit(in_name, in_layer, geometry_descriptor));
}

void AbstractViewerWindow::commit_2d_geometry(const std::string &in_name, const float &in_layer, const std::shared_ptr<GeometryDescriptor> &geometry_descriptor)
{
    /* The Below Line 
       
       is_view_changed = true;
       enable_selection_rendering = true;

       is added because we need to update the selection Buffer with new geometries

       This might not be correct name but we need to add is_view_updated but it would be of same effect.
       check line 268 in mouse_move_event
    */
    is_view_changed = true; 
    enable_selection_rendering = true;
    /*--------------------------------------------------------------------------------------------*/
    if(in_layer == GL_LAYER_BACKGROUND_2D) 
    {
       m_2d_geometry_commit_stack.emplace_back(geometry_commit(in_name, in_layer, geometry_descriptor));
    }
    else if(in_layer == GL_LAYER_FOREGROUND_2D)
    {
       m_2d_geometry_commit_stack.emplace_front(geometry_commit(in_name, in_layer, geometry_descriptor));
    }
    else
    {
        GP_ERROR("2d Geometries can only be commited to GL_LAYER_BACKGROUND_2D or GL_LAYER_FOREGROUND_2D layers");
    }
}

void AbstractViewerWindow::upload_commits()
{
    if (!m_geometry_commit_stack.empty())
    {
        for (size_t i = m_geometry_commit_stack.size(); i > 0; --i)
        {
            auto &commit = m_geometry_commit_stack[i - 1];
            m_scene->commit_geometry(commit.name, commit.layer_id, commit.geometry);
        }

        m_geometry_commit_stack.clear();
    }

    if (!m_2d_geometry_commit_stack.empty())
    {
        for (size_t i = m_2d_geometry_commit_stack.size(); i > 0; --i)
        {
            auto &commit = m_2d_geometry_commit_stack[i - 1];
            m_scene->commit_geometry(commit.name, commit.layer_id, commit.geometry);
        }

        m_2d_geometry_commit_stack.clear();
    }
}

std::shared_ptr<GeometryDescriptor>& AbstractViewerWindow::get_geometry(const std::string &in_name)
{
    return m_scene->get_geometry(in_name);
}

void AbstractViewerWindow::hide_geometry(const std::string &in_name)
{
    m_hidden_geometries.insert(in_name);
}

void AbstractViewerWindow::show_geometry(const std::string &in_name)
{
    m_hidden_geometries.erase(in_name);
}

void AbstractViewerWindow::remove_geometry(const std::string &in_name)
{
    accquire_render_context();
    if (m_scene->has_entity(in_name) == false)
    {
        GP_TRACE("Entity not found");
        return;
    }
    else
    {
        m_scene->destroy_entity(in_name);
    }
}

void AbstractViewerWindow::destroy_entities_in_layer(const float &in_layer_id)
{
    m_scene->destroy_entities_in_layer(in_layer_id);
}

void AbstractViewerWindow::clear_display_layers()
{
    m_current_displayed_layers.clear();
}

void AbstractViewerWindow::clear_screen(const float &in_r, const float &in_g, const float &in_b, const float &in_a)
{
    accquire_render_context();
    m_scene->clear_screen(in_r, in_g, in_b, in_a);
}

void AbstractViewerWindow::add_display_layer(const float &in_layer_id)
{
    //These Special 2D layers cannot be added directly to stack due to depth and order of rendering
    if(in_layer_id == GL_LAYER_BACKGROUND_2D || in_layer_id == GL_LAYER_FOREGROUND_2D) 
    return;

    if (in_layer_id == GL_LAYER_DISPLAY_ALL)
    {
        m_current_displayed_layers.clear();
        m_current_displayed_layers.insert(GL_LAYER_DISPLAY_ALL);
        return;
    }
    m_current_displayed_layers.insert(in_layer_id);
}

void AbstractViewerWindow::remove_display_layer(const float &in_layer_id)
{
    if (in_layer_id == GL_LAYER_DISPLAY_ALL)
    {
        m_current_displayed_layers.clear();
        return;
    }
    m_current_displayed_layers.erase(in_layer_id);
}

void AbstractViewerWindow::set_bounding_box(const std::vector<double> &in_bounding_box)
{
    m_camera->set_updated_bounding_box({glm::vec3(in_bounding_box[0], in_bounding_box[1], in_bounding_box[2]),
                                        glm::vec3(in_bounding_box[3], in_bounding_box[4], in_bounding_box[5])});
    create_and_display_cor();
    create_and_display_axes();
    create_and_display_bounding_box();
    update_display();
}

void AbstractViewerWindow::set_light_position(const float &x, const float &y, const float &z)
{
   m_light_position = LightPosition(x, y, z);
   m_scene->get_scene_state().LightPosition = glm::vec3(x, y, z);
}

bool AbstractViewerWindow::is_lighting_enabled() const
{
    return m_scene->get_scene_state().enable_lighting;
}

void AbstractViewerWindow::set_lighting_state(bool in_state)
{
    m_scene->get_scene_state().enable_lighting = in_state;
}

void AbstractViewerWindow::enable_cluster_selection()
{
    is_cluster_selection_enabled = true;
}

std::unordered_set<std::string> &&AbstractViewerWindow::stop_cluster_selection()
{
    is_cluster_selection_enabled = false;
    return std::move(m_selected_entities);
}

bool AbstractViewerWindow::is_blending_enabled() const
{
    return m_scene->get_scene_state().is_blending_enabled;
}

void AbstractViewerWindow::set_blend_state(bool in_state)
{
    m_scene->get_scene_state().enable_blending = in_state;
}

void AbstractViewerWindow::set_workplane(double a, double b, double c, double d, double size)
{
    m_workplane = WorkPlane(a,b,c,d);
    m_workplane.size = size;
}

void AbstractViewerWindow::set_workplane(double x, double y, double z, double nx, double ny, double nz, double size)
{
    m_workplane = WorkPlane(x, y, z, nx, ny, nz);
    m_workplane.size = size;
}

void AbstractViewerWindow::set_zoom_ratio(const float &in_zoom_ratio)
{
    m_camera->set_zoom_ratio(in_zoom_ratio);
    create_and_display_cor();
    m_camera->processMousePan(0, 0);
    update_display();
}

void AbstractViewerWindow::snap_to_xy_plane()
{
    m_camera->snap_to_xy_plane();
    create_and_display_cor();
    m_camera->processMousePan(0, 0);
    update_display();
}

void AbstractViewerWindow::snap_to_xz_plane()
{
    m_camera->snap_to_xz_plane();
    create_and_display_cor();
    m_camera->processMousePan(0, 0);
    update_display();
}

void AbstractViewerWindow::snap_to_yz_plane()
{
    m_camera->snap_to_yz_plane();
    create_and_display_cor();
    m_camera->processMousePan(0, 0);
    update_display();
}

void AbstractViewerWindow::snap_to_yx_plane()
{
    m_camera->snap_to_yx_plane();
    create_and_display_cor();
    m_camera->processMousePan(0, 0);
    update_display();
}

void AbstractViewerWindow::snap_to_zx_plane()
{
    m_camera->snap_to_zx_plane();
    create_and_display_cor();
    m_camera->processMousePan(0, 0);
    update_display();
}

void AbstractViewerWindow::snap_to_zy_plane()
{
    m_camera->snap_to_zy_plane();
    create_and_display_cor();
    m_camera->processMousePan(0, 0);
    update_display();
}

void AbstractViewerWindow::snap_to_isometric_view()
{
    m_camera->snap_to_isometric_view();
    create_and_display_cor();
    m_camera->processMousePan(0, 0);
    update_display();
}

void AbstractViewerWindow::zoom_in()
{
    wheel_event(1.0);
}

void AbstractViewerWindow::zoom_out()
{
    wheel_event(-1.0);
}

void AbstractViewerWindow::frame_scene(const char* entity_name)
{
    m_camera->set_zoom_ratio(1.0f);
    
    if(entity_name == nullptr)
    {
        return; 
    }
    
    std::shared_ptr<GridPro_GFX::GeometryDescriptor>& entity_descriptor = get_geometry(entity_name);
    
    auto bb = (entity_descriptor->get_bounding_box());

    set_bounding_box({bb[0], bb[1], bb[2], bb[3], bb[4], bb[5]});
    
    create_and_display_cor();
    update_display();
}

void AbstractViewerWindow::create_and_display_cor()
{
    // auto sphere_desc = gp_primitives::sphere().get_descriptor("sphere_test");
    // sphere_desc->set_wireframe_mode(GL_WIREFRAME_OVERLAY);
    // sphere_desc->set_wireframe_color(0,0,0,255);
    // sphere_desc->set_fill_color(0,0,255,255);
    // commit_geometry("test_sphere",  GL_LAYER_BACKGROUND,  sphere_desc);
    std::vector<float> cor_axis_line_coords =
    {
        0.0f, 0.0f, 0.0f, // Origin
        2.0f, 0.0f, 0.0f, // X-axis
        0.0f, 0.0f, 0.0f, // Origin
        0.0f, 2.0f, 0.0f, // Y-axis
        0.0f, 0.0f, 0.0f, // Origin
        0.0f, 0.0f, 2.0f  // Z-axis
    };

    float scaling_factor = m_camera->get_bounding_box().get_max_dimension()*0.3;

    for (auto &coord : cor_axis_line_coords)
    {
        coord *= scaling_factor;
    }

    glm::vec3 center_of_rotation = m_camera->get_bounding_box_center();

    glm::vec3 translation_vector = m_camera->get_world_space_translation_vector(glm::vec2(m_screen_dims.width/2, m_screen_dims.height/2), glm::vec2(m_screen_dims.width*(0.94), m_screen_dims.height*(0.1)));

    center_of_rotation += translation_vector; 
    
    float zoom_ratio = m_camera->get_zoom_ratio();

    for(auto& i : cor_axis_line_coords)
    {
        i *=  zoom_ratio;
    }

    std::vector<float> translated_axis_line_coords;

    for (int i = 0; i < cor_axis_line_coords.size(); i += 3)
    {
        translated_axis_line_coords.push_back(cor_axis_line_coords[i] + center_of_rotation.x);
        translated_axis_line_coords.push_back(cor_axis_line_coords[i + 1] + center_of_rotation.y);
        translated_axis_line_coords.push_back(cor_axis_line_coords[i + 2] + center_of_rotation.z);
    }

    std::vector<uint32_t> cor_axis_line_indices = {
        0, 1, // X-axis line
        2, 3, // Y-axis line
        4, 5  // Z-axis line
    };

    std::vector<uint8_t> cor_axis_line_colors =
    {
        255, 0, 0, 255, 0, 0,
        0, 255, 0, 0, 255, 0,
        0, 0, 255, 0, 0, 255
    };

    m_axis_descriptor->set_current_primitive_set("cor_axis_line", GL_LINES);
    m_axis_descriptor->copy_pos_array(translated_axis_line_coords);
    m_axis_descriptor->copy_index_array(cor_axis_line_indices);
    m_axis_descriptor->copy_color_array(cor_axis_line_colors);
    m_axis_descriptor->set_color_scheme(GL_COLOR_PER_VERTEX);
    m_axis_descriptor->set_line_width(40.0f/(device_pixel_ratio()*2));

    commit_geometry("cor_axis_line", GL_LAYER_BACKGROUND, m_axis_descriptor);

    // Display the center of rotation
    glm::vec3 center_of_rotation_coords = m_camera->get_current_center_of_rotation();

    std::vector<float> center_of_rotation_axis_line_coords;
    
    for (int i = 0; i < cor_axis_line_coords.size(); i += 3)
    {
        center_of_rotation_axis_line_coords.push_back(cor_axis_line_coords[i] / 2.0f + center_of_rotation_coords.x);
        center_of_rotation_axis_line_coords.push_back(cor_axis_line_coords[i + 1] / 2.0f + center_of_rotation_coords.y);
        center_of_rotation_axis_line_coords.push_back(cor_axis_line_coords[i + 2] / 2.0f + center_of_rotation_coords.z);
    }

    m_center_of_rotation_descriptor->set_current_primitive_set("center_of_rotation_axis_line", GL_LINES);
    m_center_of_rotation_descriptor->copy_pos_array(center_of_rotation_axis_line_coords);
    m_center_of_rotation_descriptor->move_color_array(std::move(cor_axis_line_colors));
    m_center_of_rotation_descriptor->move_index_array(std::move(cor_axis_line_indices));
    m_center_of_rotation_descriptor->set_color_scheme(GL_COLOR_PER_VERTEX);
    m_center_of_rotation_descriptor->set_line_width(2.0f);
    commit_geometry("center_of_rotation_axis_line", GL_LAYER_BACKGROUND, m_center_of_rotation_descriptor);
 
    // Rendering the End Points of reference axis
    std::vector<uint32_t> cor_axis_end_points_indices = 
    {
         1, // X-axis line
         3, // Y-axis line
         5  // Z-axis line
    };

    std::vector<uint8_t> cor_axis_end_points_colors =
    {
        255, 0, 0, 255, 0, 0,
        0, 255, 0, 0, 255, 0,
        0, 0, 255, 0, 0, 255
    };

    m_axis_end_point_descriptor->set_current_primitive_set("axis_end_points", GL_POINTS);
    m_axis_end_point_descriptor->copy_pos_array(translated_axis_line_coords);
    m_axis_end_point_descriptor->copy_index_array(cor_axis_end_points_indices);
    m_axis_end_point_descriptor->copy_color_array(cor_axis_end_points_colors);
    m_axis_end_point_descriptor->set_color_scheme(GL_COLOR_PER_VERTEX);
    m_axis_end_point_descriptor->set_point_size(20.0f);
    commit_geometry("axis_end_points", GL_LAYER_BACKGROUND, m_axis_end_point_descriptor);
    
    struct Point { float x, y, z; };
    std::vector<Point> xyz_points(3);

    uint32_t i = 0;

    for(auto& p : xyz_points)
    {
       p.x = translated_axis_line_coords[cor_axis_end_points_indices[i]*3];
       p.y = translated_axis_line_coords[cor_axis_end_points_indices[i]*3 + 1];
       p.z = translated_axis_line_coords[cor_axis_end_points_indices[i]*3 + 2];
       ++i;
    }
    
    for(auto& p : xyz_points)
    {
       
    }

}

void AbstractViewerWindow::create_and_display_axes()
{
    return;
    // Display the origin axis
    std::vector<float> origin_axis_line_coords =
    {
        0.0f, 0.0f, 0.0f,  // Origin
        1.0f, 0.0f, 0.0f,  // X-axis
        0.0f, 0.0f, 0.0f,  // Origin
        0.0f, 1.0f, 0.0f,  // Y-axis
        0.0f, 0.0f, 0.0f,  // Origin
        0.0f, 0.0f, 1.0f,  // Z-axis
        0.0f, 0.0f, 0.0f,  // Origin
       -1.0f, 0.0f, 0.0f, // -X-axis
        0.0f, 0.0f, 0.0f,  // Origin
        0.0f,-1.0f, 0.0f, // -Y-axis
        0.0f, 0.0f, 0.0f,  // Origin
        0.0f, 0.0f,-1.0f  // -Z-axis
    };

    for (auto &coord : origin_axis_line_coords)
    {
        coord *= 1000000;
    }

    std::vector<uint8_t> origin_axis_colors =
    {
        255, 0, 0, 255, 0, 0,
        50, 150, 0, 50, 150, 0,
        0, 0, 255, 0, 0, 255,
        255, 0, 0, 255, 0, 0,
        50, 150, 0, 50, 150, 0,
        0, 0, 255, 0, 0, 255
    };

    m_center_descriptor->set_current_primitive_set("origin_axis", GL_LINES);
    m_center_descriptor->move_pos_array(std::move(origin_axis_line_coords));
    m_center_descriptor->move_color_array(std::move(origin_axis_colors));
    m_center_descriptor->set_color_scheme(GL_COLOR_PER_VERTEX);

    // m_center_descriptor->set_fill_color(255, 255, 255, 255);
    commit_geometry("origin_axis", GL_LAYER_BACKGROUND, m_center_descriptor);
}

void AbstractViewerWindow::create_and_display_bounding_box()
{
    return;
    std::array<glm::vec3, 8> bounding_box_coords = m_camera->get_bounding_box().get_bounding_box_nodes();
    std::vector<float> bounding_box_coords_vec;

    for (auto &coord : bounding_box_coords)
    {
        bounding_box_coords_vec.push_back(coord.x);
        bounding_box_coords_vec.push_back(coord.y);
        bounding_box_coords_vec.push_back(coord.z);
    }

    std::vector<uint32_t> bounding_box_indices =
    {
        0, 1, 1, 3, 3, 2, 2, 0,
        0, 4, 1, 5, 3, 7, 2, 6,
        4, 5, 5, 7, 7, 6, 6, 4
    };

    m_bounding_box_descriptor->set_current_primitive_set("bounding_box", GL_LINES);
    m_bounding_box_descriptor->move_pos_array(std::move(bounding_box_coords_vec));
    m_bounding_box_descriptor->move_index_array(std::move(bounding_box_indices));
    m_bounding_box_descriptor->set_fill_color(100, 200, 255, 255);
    m_bounding_box_descriptor->set_node_manipulator(true);
    m_bounding_box_descriptor->set_pick_scheme(GL_PICK_BY_VERTEX);
    commit_geometry("bounding_box", GL_LAYER_BACKGROUND, m_bounding_box_descriptor);
}


void AbstractViewerWindow::create_and_display_light()
{
    return;
    /// Display Light as an COR axes with Green Color
    std::vector<float> light_axis_line_coords =
    {
        m_light_position.x, m_light_position.y, m_light_position.z, // Origin
        m_light_position.x + 1.0f, m_light_position.y, m_light_position.z, // X-axis
        m_light_position.x, m_light_position.y, m_light_position.z, // Origin
        m_light_position.x, m_light_position.y + 1.0f, m_light_position.z, // Y-axis
        m_light_position.x, m_light_position.y, m_light_position.z, // Origin
        m_light_position.x, m_light_position.y, m_light_position.z + 1.0f  // Z-axis
    };

    std::vector<uint32_t> light_axis_line_indices = {
        0, 1, // X-axis line
        2, 3, // Y-axis line
        4, 5  // Z-axis line
    };

    std::vector<uint8_t> light_axis_line_colors =
    {
        0, 255, 0, 0, 255, 0,
        255, 25, 0, 255, 25, 0,
        0, 25, 255, 0, 25, 255
    };

    m_light_descriptor->set_current_primitive_set("light_axis_line", GL_LINES);
    m_light_descriptor->move_pos_array(std::move(light_axis_line_coords));
    m_light_descriptor->move_index_array(std::move(light_axis_line_indices));
    m_light_descriptor->move_color_array(std::move(light_axis_line_colors));
    m_light_descriptor->set_color_scheme(GL_COLOR_PER_VERTEX);
    commit_geometry("light_axis_line", GL_LAYER_BACKGROUND, m_light_descriptor);
}

void AbstractViewerWindow::create_and_display_node_manipulator(const float& x, const float& y, const float& z)
{
    return;
    // Create an Axis for the Node Manipulator
    glm::vec3 node_position = glm::vec3(x, y, z);
    std::vector<float> node_manipulator_axis_line_coords =
    {
        node_position.x, node_position.y, node_position.z, // Origin
        node_position.x + 1.0f, node_position.y, node_position.z, // X-axis
        node_position.x, node_position.y, node_position.z, // Origin
        node_position.x, node_position.y + 1.0f, node_position.z, // Y-axis
        node_position.x, node_position.y, node_position.z, // Origin
        node_position.x, node_position.y, node_position.z + 1.0f  // Z-axis
    };

    std::vector<uint32_t> node_manipulator_axis_line_indices = {
        0, 1, // X-axis line
        2, 3, // Y-axis line
        4, 5  // Z-axis line
    };

    std::vector<uint8_t> node_manipulator_axis_line_colors =
    {
        255, 0, 0, 255, 0, 0,
        0, 255, 0, 0, 255, 0,
        0, 0, 255, 0, 0, 255
    };

    auto node_manipulator_descriptor = std::make_shared<GeometryDescriptor>();
    
    node_manipulator_descriptor->set_current_primitive_set("node_manipulator_axis_line", GL_LINES);
    node_manipulator_descriptor->move_pos_array(std::move(node_manipulator_axis_line_coords));
    node_manipulator_descriptor->move_index_array(std::move(node_manipulator_axis_line_indices));
    node_manipulator_descriptor->move_color_array(std::move(node_manipulator_axis_line_colors));
    node_manipulator_descriptor->set_color_scheme(GL_COLOR_PER_VERTEX);
    commit_geometry("node_manipulator_axis_line", GL_LAYER_BACKGROUND, node_manipulator_descriptor);
}

void AbstractViewerWindow::create_and_display_2d_background_quad()
{
    return;
    // std::vector<float> quad_coords =
    // {
    //     -1000.0f, -1000.0f, -1000.0f, // Bottom Left
    //      1000.0f, -1000.0f, -1000.0f,  // Bottom Right
    //      1000.0f,  1000.0f, -1000.0f,   // Top Right
    //     -1000.0f,  1000.0f, -1000.0f   // Top Left
    // };

    std::vector<float> quad_coords =
    {
        -1.0f, -1.0f,  0.0f, // Bottom Left
         1.0f, -1.0f,  0.0f,  // Bottom Right
         1.0f,  1.0f,  0.0f,   // Top Right
        -1.0f,  1.0f,  0.0f   // Top Left
    };

    // Four Colors for the Quad
    std::vector<uint8_t> quad_colors =
    {
        30, 30, 30,
        30, 30, 30,
        30, 30, 30,
        30, 30, 30
    };
         
    m_2d_background_quad_descriptor->set_current_primitive_set("2d_background_quad", GL_QUADS);
    m_2d_background_quad_descriptor->move_pos_array(std::move(quad_coords));
    m_2d_background_quad_descriptor->move_color_array(std::move(quad_colors));
    m_2d_background_quad_descriptor->set_color_scheme(GL_COLOR_PER_VERTEX);
    commit_2d_geometry("2d_background_quad", GL_LAYER_BACKGROUND_2D, m_2d_background_quad_descriptor);
}


void AbstractViewerWindow::create_and_display_mouse_ray()
{
    return;
    Ray ray = m_camera->generate_mouse_ray(glm::vec2(m_prev_mouse_state.x, m_prev_mouse_state.y));
    
    glm::vec3 ray_origin = ray.origin();

    glm::vec3 ray_direction = ray.direction();
    
    glm::vec3 ray_target =  ray.point_at_parameter(1000.0f);

    std::vector<float> ray_coords =
    {
        ray_origin.x, ray_origin.y, ray_origin.z, // Target
        0.0f, 0.0f, 0.0f 
        //ray_target.x, ray_target.y, ray_target.z // Ray
    };

    auto ray_descriptor = std::make_shared<GeometryDescriptor>();
    ray_descriptor->set_current_primitive_set("mouse_ray", GL_LINES);
    ray_descriptor->move_pos_array(std::move(ray_coords));
    ray_descriptor->set_fill_color(255, 0, 0, 255);
    ray_descriptor->set_line_width(2.0f);
    commit_geometry("mouse_ray", GL_LAYER_BACKGROUND, ray_descriptor);
}

void AbstractViewerWindow::create_and_display_workplane()
{
    if(!is_workplane_active || !m_workplane.is_valid()) return;

    std::vector<float> quad_coords = m_workplane.get_quad();
    quad_coords.push_back(quad_coords[0]);
    quad_coords.push_back(quad_coords[1]);
    quad_coords.push_back(quad_coords[2]);

    m_workplane_descriptor->set_current_primitive_set("viewer_workplane_quad", GL_LINE_STRIP);
    m_workplane_descriptor->move_pos_array(std::move(quad_coords));
    m_workplane_descriptor->set_fill_color(255, 255, 255, 255);
    m_workplane_descriptor->set_line_width(3.0f);
    commit_geometry("viewer_workplane_quad", GL_LAYER_BACKGROUND, m_workplane_descriptor);
}

void AbstractViewerWindow::create_and_display_gizmo()
{
    m_gizmo_handles["default"] = GizmoHandle::create("default");
    auto m_gizmo = m_gizmo_handles["default"];
    m_gizmo->update_from_bbox(m_camera->get_bounding_box().min*10.0f, m_camera->get_bounding_box().max*10.0f);
    
    // Commit to the renderer
    commit_geometry(m_gizmo->get_translate_name(), GL_LAYER_1, m_gizmo->get_translate_descriptor());
    commit_geometry(m_gizmo->get_scale_name(), GL_LAYER_1, m_gizmo->get_scale_descriptor());
    commit_geometry(m_gizmo->get_rotate_name(), GL_LAYER_1, m_gizmo->get_rotate_descriptor());

}

AbstractViewerWindow::WorkPlane::WorkPlane(double px, double py, double pz, double nx, double ny, double nz)
{
    center = {0.0f, 0.0f, 0.0f};
    double norm = std::sqrt(nx * nx + ny * ny + nz * nz);
    if (norm == 0.0)
    {
        a = b = c = d = 0.0;
        return;
    }

    a = nx / norm;
    b = ny / norm;
    c = nz / norm;

    d = -(a * px + b * py + c * pz);
}

AbstractViewerWindow::Point AbstractViewerWindow::WorkPlane::normal() const
{
    double mag = std::sqrt(a * a + b * b + c * c);
    return (mag == 0) ? Point{0, 0, 0} : Point{a / mag, b / mag, c / mag};
}

std::vector<float> AbstractViewerWindow::WorkPlane::get_quad() const
{
    Point n = normal();

    Point tangent = (std::abs(n.x) < 0.9) ?  Point{1, 0, 0} : Point{0, 1, 0};

    double dot = n.x * tangent.x + n.y * tangent.y + n.z * tangent.z;
    tangent = 
    {
        tangent.x - dot * n.x,
        tangent.y - dot * n.y,
        tangent.z - dot * n.z
    };

    double t_mag = std::sqrt(tangent.x*tangent.x + tangent.y*tangent.y + tangent.z*tangent.z);
    tangent = { tangent.x / t_mag, tangent.y / t_mag, tangent.z / t_mag };

    Point bitangent = 
    {
        n.y * tangent.z - n.z * tangent.y,
        n.z * tangent.x - n.x * tangent.z,
        n.x * tangent.y - n.y * tangent.x
    };

    double half = size / 2.0;
    std::vector<Point> corners = 
    {
        { -half * tangent.x - half * bitangent.x,
          -half * tangent.y - half * bitangent.y,
          -half * tangent.z - half * bitangent.z },

        {  half * tangent.x - half * bitangent.x,
           half * tangent.y - half * bitangent.y,
           half * tangent.z - half * bitangent.z },

        {  half * tangent.x + half * bitangent.x,
           half * tangent.y + half * bitangent.y,
           half * tangent.z + half * bitangent.z },

        { -half * tangent.x + half * bitangent.x,
          -half * tangent.y + half * bitangent.y,
          -half * tangent.z + half * bitangent.z }
    };

    std::vector<float> quad_vertices;
    quad_vertices.reserve( corners.size() * 3 );
    for (auto& p : corners) 
    {
        quad_vertices.push_back(static_cast<float>(p.x + center.x));
        quad_vertices.push_back(static_cast<float>(p.y + center.y));
        quad_vertices.push_back(static_cast<float>(p.z + center.z));
    }

    return quad_vertices;
}

AbstractViewerWindow::WorkSpaceConfig::WorkSpaceConfig(std::string config_filename)
    : config_file_name(std::move(config_filename)),
      is_updated(false)
{
    std::ifstream in(config_file_name);
    if (!in) return; 

    nlohmann::json j;
    in >> j;
    for (auto& [key, value] : j.items()) {
        Color col;
        col.r = static_cast<uint8_t>(value.value("r", 0));
        col.g = static_cast<uint8_t>(value.value("g", 0));
        col.b = static_cast<uint8_t>(value.value("b", 0));
        col.a = static_cast<uint8_t>(value.value("a", 255));
        colors_map.emplace(key, col);
    }
}

AbstractViewerWindow::WorkSpaceConfig::~WorkSpaceConfig()
{
    if (!is_updated) return;

    nlohmann::json j;
    for (auto const& [key, col] : colors_map) {
        j[key] = {
            {"r", col.r},
            {"g", col.g},
            {"b", col.b},
            {"a", col.a}
        };
    }

    std::ofstream out(config_file_name);
    out << j.dump(4);
}

const AbstractViewerWindow::WorkSpaceConfig::Color& AbstractViewerWindow::WorkSpaceConfig::operator[](std::string object_name)
{
    static const Color default_color{0, 0, 0, 255};
    auto it = colors_map.find(object_name);
    return (it != colors_map.end() ? it->second : default_color);
}

void AbstractViewerWindow::WorkSpaceConfig::set_color(std::string object_name, Color color)
{
    colors_map[object_name] = color;
    is_updated = true;
}


// AbstractViewerWindow::Gizmo::Gizmo(bool is_translatable, bool is_rotatable, bool is_scalable)
// {

// }

// void AbstractViewerWindow::Gizmo::handle_transform_event(double trans_x, double trans_y)
// {

// }



AbstractViewerWindow::Legend::Legend()
{
   
}

void AbstractViewerWindow::Legend::handle_legend_click_or_drag_event(std::string legend_id, uint32_t quad_subentity_id)
{

}        