#pragma once

#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gp_gui_camera.h" 
#include "gp_gui_geometry_descriptor.h"

class GizmoHandle
{
public:
    enum Mode { TRANSLATE, SCALE, ROTATE };

    static std::shared_ptr<GizmoHandle> create(const std::string& name)
    {
        return std::make_shared<GizmoHandle>(name);
    }

    GizmoHandle(const std::string& name) : m_name(name)
    {
        m_translate_desc = std::make_shared<GridPro_GFX::GeometryDescriptor>();
        m_scale_desc = std::make_shared<GridPro_GFX::GeometryDescriptor>();
        m_rotate_desc = std::make_shared<GridPro_GFX::GeometryDescriptor>();
    }

    std::string get_translate_name() const { return m_name + "_translate"; }
    std::string get_scale_name()     const { return m_name + "_scale"; }
    std::string get_rotate_name()    const { return m_name + "_rotate"; }

    void update_from_bbox(const glm::vec3 &min, const glm::vec3 &max)
    {
        m_center = (min + max) * 0.5f;
        glm::vec3 dims = max - min;
        m_size = std::max({dims.x, dims.y, dims.z}) * 0.6f;

        build_translation_and_scale();
        build_rotation_rings();
    }

    /**
     * Calculates the delta vector based on 2D mouse movement.
     */
    glm::vec3 ProcessMouseMoveEvent(float delX, float delY, int axis_id, GridPro_GFX::OrthographicCamera* camera)
    {
        if (axis_id < 0 || axis_id > 2) return glm::vec3(0.0f);

        glm::vec3 world_axis(0.0f);
        world_axis[axis_id] = 1.0f;

        // Orthographic mapping: Calculate units per pixel based on zoom and bounding box max dimension
        float units_per_pixel = (camera->get_zoom_ratio() * camera->get_bounding_box().get_max_dimension()) / 800.0f;

        glm::vec3 cam_right = glm::normalize(glm::cross(camera->get_camera_orientation(), camera->get_camera_updirection()));
        glm::vec3 cam_up    = camera->get_camera_updirection();
        
        glm::vec3 world_delta = (cam_right * (delX * units_per_pixel)) + (cam_up * (-delY * units_per_pixel)); 

        float magnitude = glm::dot(world_delta, world_axis);
        return world_axis * magnitude;
    }

    /**
     * Converts the interaction result into a 4x4 Transform Matrix.
     * @param mode: TRANSLATE, SCALE, or ROTATE
     * @param deltaVec: The vector returned by ProcessMouseMoveEvent
     */
    glm::mat4 GetTransformMatrix(Mode mode, const glm::vec3& deltaVec)
    {
        glm::mat4 transform = glm::mat4(1.0f);

        switch (mode)
        {
            case Mode::TRANSLATE:
                // Simple translation matrix
                transform = glm::translate(transform, deltaVec);
                break;

            case Mode::SCALE:
            {
                // Convert delta movement to a scale factor (e.g., moving 1 unit adds 10% scale)
                float scaleFac = 1.0f + (glm::length(deltaVec) * (glm::dot(glm::normalize(deltaVec + 1e-6f), deltaVec) > 0 ? 1.0f : -1.0f) / m_size);
                // Apply uniform scale or axis-aligned scale
                glm::vec3 scaleVec(1.0f);
                if (glm::length(deltaVec) > 0) {
                    // If deltaVec is (0.1, 0, 0), it scales only X
                    scaleVec = glm::vec3(1.0f) + (deltaVec / m_size);
                }
                transform = glm::scale(transform, scaleVec);
                break;
            }

            case Mode::ROTATE:
            {
                // For rotation, the magnitude of deltaVec represents the "pull" distance converted to radians
                float angle = glm::length(deltaVec) / m_size;
                glm::vec3 axis = glm::normalize(deltaVec);
                if (glm::length(deltaVec) > 1e-6) {
                    transform = glm::rotate(transform, angle, axis);
                }
                break;
            }
        }
        return transform;
    }

    // Getters for Descriptors
    std::shared_ptr<GridPro_GFX::GeometryDescriptor> get_translate_descriptor() const { return m_translate_desc; }
    std::shared_ptr<GridPro_GFX::GeometryDescriptor> get_scale_descriptor()     const { return m_scale_desc; }
    std::shared_ptr<GridPro_GFX::GeometryDescriptor> get_rotate_descriptor()    const { return m_rotate_desc; }

private:
    void build_translation_and_scale()
    {
        std::vector<float> trans_coords = {
            m_center.x, m_center.y, m_center.z,   m_center.x + m_size, m_center.y, m_center.z,
            m_center.x, m_center.y, m_center.z,   m_center.x, m_center.y + m_size, m_center.z,
            m_center.x, m_center.y, m_center.z,   m_center.x, m_center.y, m_center.z + m_size
        };

        std::vector<uint8_t> trans_colors = {
            255, 0, 0, 
            255, 0, 0, 
            0, 255, 0, 
            0, 255, 0, 
            0, 0, 255,
            0, 0, 255
        };

        m_translate_desc->set_current_primitive_set(get_translate_name(), GL_LINES);
        m_translate_desc->copy_pos_array(trans_coords);
        m_translate_desc->copy_color_array(trans_colors);
        m_translate_desc->set_color_scheme(GL_COLOR_PER_VERTEX);
        m_translate_desc->set_pick_scheme(GL_PICK_BY_PRIMITIVE);
       // m_translate_desc->set_hover_highlightable_mode(true);
        m_translate_desc->set_line_width(3.0f);

        std::vector<float> scale_coords = {
            m_center.x + m_size, m_center.y, m_center.z,
            m_center.x, m_center.y + m_size, m_center.z,
            m_center.x, m_center.y, m_center.z + m_size
        };

        std::vector<uint8_t> scale_colors = { 255, 0, 0, 
                                              0, 255, 0, 
                                              0, 0, 255 };

        m_scale_desc->set_current_primitive_set(get_scale_name(), GL_POINTS);
        m_scale_desc->copy_pos_array(scale_coords);
        m_scale_desc->copy_color_array(scale_colors);
        m_scale_desc->set_color_scheme(GL_COLOR_PER_VERTEX);
        m_scale_desc->set_point_size(15.0f);
        // m_scale_desc->set_hover_highlightable_mode(true);
        m_scale_desc->set_pick_scheme(GL_PICK_BY_VERTEX);
    }

    void build_rotation_rings()
    {
        std::vector<float> ring_coords;
        std::vector<uint8_t> ring_colors;
        std::vector<uint32_t> ring_indices;
        const int segments = 48;
        const float PI = 3.1415926535f;

        auto add_axis_ring = [&](int axis_type) {
            uint32_t start_idx = static_cast<uint32_t>(ring_coords.size() / 3);
            for (int i = 0; i <= segments; ++i) {
                float angle = (2.0f * PI * i) / segments;
                float s = std::sin(angle) * m_size;
                float c = std::cos(angle) * m_size;

                if (axis_type == 0) ring_coords.insert(ring_coords.end(), {m_center.x, m_center.y + c, m_center.z + s});
                else if (axis_type == 1) ring_coords.insert(ring_coords.end(), {m_center.x + c, m_center.y, m_center.z + s});
                else ring_coords.insert(ring_coords.end(), {m_center.x + c, m_center.y + s, m_center.z});

                ring_colors.insert(ring_colors.end(), {(axis_type == 0 ? (uint8_t)255 : (uint8_t)0), (axis_type == 1 ? (uint8_t)255 : (uint8_t)0), (axis_type == 2 ? (uint8_t)255 : (uint8_t)0)});
                if (i < segments) {
                    ring_indices.push_back(start_idx + i);
                    ring_indices.push_back(start_idx + i + 1);
                }
            }
        };

        add_axis_ring(0); add_axis_ring(1); add_axis_ring(2);

        m_rotate_desc->set_current_primitive_set(get_rotate_name(), GL_LINES);
        m_rotate_desc->copy_pos_array(ring_coords);
        m_rotate_desc->copy_index_array(ring_indices);
        m_rotate_desc->copy_color_array(ring_colors);
        m_rotate_desc->set_color_scheme(GL_COLOR_PER_VERTEX);
        m_rotate_desc->set_pick_scheme(GL_PICK_BY_PRIMITIVE);
        // m_rotate_desc->set_hover_highlightable_mode(true);
    }

    std::string m_name;
    glm::vec3 m_center{0.0f};
    float m_size{1.0f};
    std::shared_ptr<GridPro_GFX::GeometryDescriptor> m_translate_desc, m_scale_desc, m_rotate_desc;
};