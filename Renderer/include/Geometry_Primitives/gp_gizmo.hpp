#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>

#include "gp_gui_geometry_descriptor.h"

class GizmoHandle {
public:
    GizmoHandle() {
        m_translate_desc = std::make_shared<GridPro_GFX::GeometryDescriptor>();
        m_scale_desc = std::make_shared<GridPro_GFX::GeometryDescriptor>();
        m_rotate_desc = std::make_shared<GridPro_GFX::GeometryDescriptor>();
    }

    /**
     * Updates the GizmoHandle geometry based on the provided bounding box.
     */
    void update_from_bbox(const glm::vec3& min, const glm::vec3& max) {
        // 1. Calculate Center and Scale
        m_center = (min + max) * 0.5f;
        
        glm::vec3 dims = max - min;
        // GizmoHandle size is 60% of the largest dimension for visibility
        m_size = std::max({dims.x, dims.y, dims.z}) * 0.6f;

        // 2. Build Geometry Sets
        build_translation_and_scale();
        build_rotation_rings();
    }

    // Getters for the viewer's commit_geometry calls
    std::shared_ptr<GridPro_GFX::GeometryDescriptor> get_translate_descriptor() const { return m_translate_desc; }
    std::shared_ptr<GridPro_GFX::GeometryDescriptor> get_scale_descriptor()     const { return m_scale_desc; }
    std::shared_ptr<GridPro_GFX::GeometryDescriptor> get_rotate_descriptor()    const { return m_rotate_desc; }

private:
    void build_translation_and_scale() {
        // Vertex 0: Center, 1: X-tip, 2: Y-tip, 3: Z-tip
        std::vector<float> coords = {
            m_center.x, m_center.y, m_center.z,
            m_center.x + m_size, m_center.y, m_center.z,
            m_center.x, m_center.y + m_size, m_center.z,
            m_center.x, m_center.y, m_center.z + m_size
        };

        std::vector<uint8_t> colors = {
            255, 255, 255, 255, // Center: White
            255, 0, 0, 255,     // X: Red
            0, 255, 0, 255,     // Y: Green
            0, 0, 255, 255      // Z: Blue
        };

        // --- Translation Lines ---
        std::vector<uint32_t> line_indices = {0, 1, 0, 2, 0, 3};
        m_translate_desc->set_current_primitive_set("GizmoHandle_translate_lines", GL_LINES);
        m_translate_desc->copy_pos_array(coords);
        m_translate_desc->copy_index_array(line_indices);
        m_translate_desc->copy_color_array(colors);
        m_translate_desc->set_color_scheme(GL_COLOR_PER_VERTEX);
        m_translate_desc->set_pick_scheme(GL_PICK_GEOMETRY);
        m_translate_desc->set_line_width(3.0f);

        // --- Scale Endpoints (Handles at the tips of the lines) ---
        std::vector<uint32_t> point_indices = {1, 2, 3};
        m_scale_desc->set_current_primitive_set("GizmoHandle_scale_points", GL_POINTS);
        m_scale_desc->copy_pos_array(coords);
        m_scale_desc->copy_index_array(point_indices);
        m_scale_desc->copy_color_array(colors);
        m_scale_desc->set_color_scheme(GL_COLOR_PER_VERTEX);
        m_scale_desc->set_point_size(15.0f);
        m_scale_desc->set_pick_scheme(GL_PICK_BY_VERTEX);
    }

    void build_rotation_rings() {
        std::vector<float> ring_coords;
        std::vector<uint8_t> ring_colors;
        std::vector<uint32_t> ring_indices;
        
        const int segments = 48; // Higher segment count for smoother rings
        const float PI = 3.1415926535f;

        auto add_axis_ring = [&](int axis_type) {
            uint32_t start_idx = static_cast<uint32_t>(ring_coords.size() / 3);
            for (int i = 0; i <= segments; ++i) {
                float angle = (2.0f * PI * i) / segments;
                float s = std::sin(angle) * m_size;
                float c = std::cos(angle) * m_size;

                if (axis_type == 0) { // X-Axis (YZ Plane)
                    ring_coords.insert(ring_coords.end(), {m_center.x, m_center.y + c, m_center.z + s});
                    ring_colors.insert(ring_colors.end(), {255, 0, 0, 255});
                } else if (axis_type == 1) { // Y-Axis (XZ Plane)
                    ring_coords.insert(ring_coords.end(), {m_center.x + c, m_center.y, m_center.z + s});
                    ring_colors.insert(ring_colors.end(), {0, 255, 0, 255});
                } else { // Z-Axis (XY Plane)
                    ring_coords.insert(ring_coords.end(), {m_center.x + c, m_center.y + s, m_center.z});
                    ring_colors.insert(ring_colors.end(), {0, 0, 255, 255});
                }

                // Indices for line segments
                if (i < segments) {
                    ring_indices.push_back(start_idx + i);
                    ring_indices.push_back(start_idx + i + 1);
                }
            }
        };

        add_axis_ring(0); // X Ring
        add_axis_ring(1); // Y Ring
        add_axis_ring(2); // Z Ring

        m_rotate_desc->set_current_primitive_set("GizmoHandle_rotation_rings", GL_LINES);
        m_rotate_desc->copy_pos_array(ring_coords);
        m_rotate_desc->copy_index_array(ring_indices);
        m_rotate_desc->copy_color_array(ring_colors);
        m_rotate_desc->set_color_scheme(GL_COLOR_PER_VERTEX);
        m_rotate_desc->set_pick_scheme(GL_PICK_BY_PRIMITIVE);
    }

    glm::vec3 m_center{0.0f};
    float m_size{1.0f};

    std::shared_ptr<GridPro_GFX::GeometryDescriptor> m_translate_desc;
    std::shared_ptr<GridPro_GFX::GeometryDescriptor> m_scale_desc;
    std::shared_ptr<GridPro_GFX::GeometryDescriptor> m_rotate_desc;
};