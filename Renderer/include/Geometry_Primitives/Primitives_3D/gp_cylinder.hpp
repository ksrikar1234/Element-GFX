#ifndef __GP_CYLINDER_HPP__
#define __GP_CYLINDER_HPP__

#include <vector>
#include <memory>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "gp_gui_geometry_descriptor.h"
#include "../gp_shape.hpp"

namespace gp_primitives
{
    class cylinder : public shape
    {
    public:
        /// Construct a cylinder centered at `center`, with given `radius` and `height`,
        /// subdivided into `radialRes` segments around, and `heightRes` along its height.
        cylinder(glm::vec3 center = glm::vec3(0.0f),
                 double radius = 1.0,
                 double height = 1.0,
                 uint32_t radialRes = 30,
                 uint32_t heightRes = 1)
            : m_center(center), m_radius(radius),
              m_height(height), m_radialRes(radialRes),
              m_heightRes(heightRes), m_filled(true)
        {
        }

        // Setters
        cylinder &set_center(const glm::vec3& c)
        {
            m_center = c;
            return *this;
        }
        cylinder &set_radius(double r)
        {
            m_radius = r;
            return *this;
        }
        cylinder &set_height(double h)
        {
            m_height = h;
            return *this;
        }
        cylinder &set_resolution(uint32_t radial, uint32_t vert)
        {
            m_radialRes = radial;
            m_heightRes = vert;
            return *this;
        }
        cylinder &set_fill(bool filled)
        {
            m_filled = filled;
            return *this;
        }

        // Dimensionality
        uint8_t dim() const override { return 3; }

        // Build GeometryDescriptor
        std::shared_ptr<GridPro_GFX::GeometryDescriptor>
        get_descriptor(std::string name) const 
        {
            auto desc = std::make_shared<GridPro_GFX::GeometryDescriptor>();
            if (m_filled)
            {
                desc->set_current_primitive_set(name, GL_TRIANGLES);
                desc->move_pos_array(std::move(get_mesh_coords()));
            }
            else
            {
                desc->set_current_primitive_set(name, GL_LINES);
                desc->move_pos_array(std::move(get_wireframe_coords()));
            }
            return desc;
        }

    private:
        // Triangle mesh: sides + top & bottom caps
        std::vector<float> get_mesh_coords() const
        {
            std::vector<float> coords;
            coords.reserve((m_radialRes * m_heightRes * 2 + 2 * (m_radialRes + 1)) * 3 * 3);

            // Side surface
            for (uint32_t i = 0; i < m_radialRes; ++i)
            {
                float th0 = 2.0f * M_PI * i / m_radialRes;
                float th1 = 2.0f * M_PI * (i + 1) / m_radialRes;
                for (uint32_t j = 0; j < m_heightRes; ++j)
                {
                    float y0 = -m_height / 2 + m_height * float(j) / m_heightRes;
                    float y1 = -m_height / 2 + m_height * float(j + 1) / m_heightRes;

                    glm::vec3 p00 = m_center + glm::vec3(m_radius * cos(th0), y0, m_radius * sin(th0));
                    glm::vec3 p01 = m_center + glm::vec3(m_radius * cos(th0), y1, m_radius * sin(th0));
                    glm::vec3 p10 = m_center + glm::vec3(m_radius * cos(th1), y0, m_radius * sin(th1));
                    glm::vec3 p11 = m_center + glm::vec3(m_radius * cos(th1), y1, m_radius * sin(th1));

                    // triangle 1: p00, p10, p11
                    for (auto &v : {p00, p10, p11})
                    {
                        coords.push_back(v.x);
                        coords.push_back(v.y);
                        coords.push_back(v.z);
                    }
                    // triangle 2: p00, p11, p01
                    for (auto &v : {p00, p11, p01})
                    {
                        coords.push_back(v.x);
                        coords.push_back(v.y);
                        coords.push_back(v.z);
                    }
                }
            }

            // Top cap (triangle fan)
            glm::vec3 topCenter = m_center + glm::vec3(0.0f, m_height / 2, 0.0f);
            coords.push_back(topCenter.x);
            coords.push_back(topCenter.y);
            coords.push_back(topCenter.z);
            for (uint32_t i = 0; i <= m_radialRes; ++i)
            {
                float theta = 2.0f * M_PI * i / m_radialRes;
                glm::vec3 p = m_center + glm::vec3(m_radius * cos(theta), m_height / 2, m_radius * sin(theta));
                coords.push_back(p.x);
                coords.push_back(p.y);
                coords.push_back(p.z);
            }

            // Bottom cap (triangle fan)
            glm::vec3 botCenter = m_center + glm::vec3(0.0f, -m_height / 2, 0.0f);
            coords.push_back(botCenter.x);
            coords.push_back(botCenter.y);
            coords.push_back(botCenter.z);
            for (uint32_t i = 0; i <= m_radialRes; ++i)
            {
                float theta = 2.0f * M_PI * i / m_radialRes;
                glm::vec3 p = m_center + glm::vec3(m_radius * cos(theta), -m_height / 2, m_radius * sin(theta));
                coords.push_back(p.x);
                coords.push_back(p.y);
                coords.push_back(p.z);
            }

            return coords;
        }

        // Wireframe: circular edges + vertical lines
        std::vector<float> get_wireframe_coords() const
        {
            std::vector<float> coords;
            coords.reserve((2 * (m_radialRes + 1) + 2 * m_radialRes) * 3);

            // Round edges
            for (uint32_t i = 0; i <= m_radialRes; ++i)
            {
                float theta = 2.0f * M_PI * i / m_radialRes;
                glm::vec3 top = m_center + glm::vec3(m_radius * cos(theta), m_height / 2, m_radius * sin(theta));
                glm::vec3 bot = m_center + glm::vec3(m_radius * cos(theta), -m_height / 2, m_radius * sin(theta));
                coords.push_back(top.x);
                coords.push_back(top.y);
                coords.push_back(top.z);
                coords.push_back(bot.x);
                coords.push_back(bot.y);
                coords.push_back(bot.z);
            }

            // Vertical lines
            for (uint32_t i = 0; i < m_radialRes; ++i)
            {
                float theta = 2.0f * M_PI * i / m_radialRes;
                glm::vec3 top = m_center + glm::vec3(m_radius * cos(theta), m_height / 2, m_radius * sin(theta));
                glm::vec3 bot = m_center + glm::vec3(m_radius * cos(theta), -m_height / 2, m_radius * sin(theta));
                coords.push_back(top.x);
                coords.push_back(top.y);
                coords.push_back(top.z);
                coords.push_back(bot.x);
                coords.push_back(bot.y);
                coords.push_back(bot.z);
            }

            return coords;
        }

        glm::vec3 m_center;
        double m_radius;
        double m_height;
        uint32_t m_radialRes;
        uint32_t m_heightRes;
        bool m_filled;
    };

} // namespace gp_primitives

#endif // __GP_CYLINDER_HPP__
