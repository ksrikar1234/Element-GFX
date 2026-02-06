#ifndef __GP_CIRCLE_HPP__
#define __GP_CIRCLE_HPP__

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "gp_gui_geometry_descriptor.h"

#include "../gp_shape.hpp"

namespace gp_primitives
{
    class circle : public shape
    {
    public:
        circle(glm::vec3 center = glm::vec3(0.0f), double radius = 1.0, uint32_t in_res = 500, glm::vec3 in_normal = glm::vec3(0.0f, 0.0f, 1.0f))
            : m_center(center), m_radius(radius), m_resolution(in_res), m_normal(in_normal), is_filled(false)
        {}

        std::shared_ptr<GridPro_GFX::GeometryDescriptor> get_descriptor(std::string desc_name) const override
        {
            auto circle_descriptor = std::make_shared<GridPro_GFX::GeometryDescriptor>();
            if(is_filled)
            {
                circle_descriptor->set_current_primitive_set(desc_name, GL_TRIANGLE_FAN);
                circle_descriptor->move_pos_array(std::move(get_filled_triangle_fan_coords()));
            }
            else
            {
                circle_descriptor->set_current_primitive_set(desc_name, GL_LINE_STRIP);
                circle_descriptor->move_pos_array(std::move(get_circumference_coords()));
            }

            return circle_descriptor;
        }

        circle& set_resolution(uint32_t res)
        {
            m_resolution = res;
            return *this;
        }

        circle& set_radius(double in_radius)
        {
            m_radius = in_radius;
            return *this;
        }

        circle& set_center(glm::vec3 in_center)
        {
            m_center = in_center;
            return *this;
        }

        circle& set_normal(glm::vec3 in_normal)
        {
            m_normal = in_normal;
            return *this;
        }

        circle& set_resolution(double res)
        {
            m_resolution = res;
            return *this;
        }

        circle& set_fill(bool filled)
        {
           is_filled = filled;
           return *this;
        }

        double radius() const
        {
            return m_radius;
        }

        glm::vec3 center() const
        {
            return m_center;
        }

        uint8_t dim() const override
        {
            return 2;
        }

        std::vector<float> get_circumference_coords() const
        {
            std::vector<float> coords;
            coords.reserve(m_resolution * 3);

            const float two_pi = 2.0f * static_cast<float>(M_PI);
            glm::vec3 default_normal(0.0f, 0.0f, 1.0f);


            glm::mat3 rotation(1.0f); 
            if (glm::normalize(m_normal) != default_normal)
            {
                glm::vec3 axis = glm::cross(default_normal, m_normal);
                float angle = acos(glm::dot(glm::normalize(default_normal), glm::normalize(m_normal)));
                if (glm::length(axis) > 0.0001f)
                {
                    axis = glm::normalize(axis);
                    rotation = glm::mat3(glm::rotate(glm::mat4(1.0f), angle, axis));
                }
            }

            for (uint32_t i = 0; i < m_resolution; ++i)
            {
                float theta = two_pi * static_cast<float>(i) / static_cast<float>(m_resolution);
                glm::vec3 point_local = glm::vec3(
                    static_cast<float>(m_radius) * cos(theta),
                    static_cast<float>(m_radius) * sin(theta),
                    0.0f);

                glm::vec3 rotated = rotation * point_local;
                glm::vec3 world_point = m_center + rotated;

                coords.push_back(world_point.x);
                coords.push_back(world_point.y);
                coords.push_back(world_point.z);
            }

            return coords;
        }

        std::vector<float> get_filled_triangle_fan_coords() const
        {
            std::vector<float> coords;
            coords.reserve((m_resolution + 2) * 3);

            glm::vec3 default_normal(0.0f, 0.0f, 1.0f);

            glm::mat3 rotation(1.0f);
            if (glm::normalize(m_normal) != default_normal)
            {
                glm::vec3 axis = glm::cross(default_normal, m_normal);
                float angle = acos(glm::dot(glm::normalize(default_normal), glm::normalize(m_normal)));
                if (glm::length(axis) > 0.0001f)
                {
                    axis = glm::normalize(axis);
                    rotation = glm::mat3(glm::rotate(glm::mat4(1.0f), angle, axis));
                }
            }

            coords.push_back(m_center.x);
            coords.push_back(m_center.y);
            coords.push_back(m_center.z);

            const float two_pi = 2.0f * static_cast<float>(M_PI);
            for (uint32_t i = 0; i <= m_resolution; ++i) // <= to close the fan
            {
                float theta = two_pi * static_cast<float>(i % m_resolution) / static_cast<float>(m_resolution);

                glm::vec3 local_point(
                    static_cast<float>(m_radius) * cos(theta),
                    static_cast<float>(m_radius) * sin(theta),
                    0.0f
                );

                glm::vec3 rotated = rotation * local_point;
                glm::vec3 world_point = m_center + rotated;

                coords.push_back(world_point.x);
                coords.push_back(world_point.y);
                coords.push_back(world_point.z);
            }

            return coords;
        }

        double radius() const
        {
            return m_radius;
        }

        glm::vec3 center() const
        {
            return m_center;
        }

    private:
        glm::vec3 m_center;
        glm::vec3 m_normal;
        double m_radius;
        uint32_t m_resolution;
        bool is_filled;
    };
}

#endif
