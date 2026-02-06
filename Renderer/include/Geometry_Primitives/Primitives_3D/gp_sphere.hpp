#ifndef __GP_SPHERE_HPP__
#define __GP_SPHERE_HPP__

#include <vector>
#include <memory>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "gp_gui_geometry_descriptor.h"
#include "../gp_shape.hpp"

namespace gp_primitives
{
    class sphere : public shape
    {
    public:
        sphere(glm::vec3 center = glm::vec3(0.0f),
               double radius = 100.0,
               uint32_t latRes = 30,
               uint32_t lonRes = 60)
            : m_center(center), m_radius(radius),
              m_latRes(latRes), m_lonRes(lonRes),
              m_filled(true)
        {
            if(radius > 1.0f) 
            {
                latRes *= static_cast<uint32_t>(radius * 4.0f);
                lonRes *= static_cast<uint32_t>(radius * 4.0f);
            }
        }

        /// Setters
        sphere& set_center(const glm::vec3 &c)
        {
            m_center = c;
            return *this;
        }
        sphere& set_radius(double r)
        {
            m_radius = r;
            return *this;
        }
        sphere& set_resolution(uint32_t lat, uint32_t lon)
        {
            m_latRes = lat;
            m_lonRes = lon;
            return *this;
        }
        sphere& set_fill(bool filled)
        {
            m_filled = filled;
            return *this;
        }

        uint8_t dim() const override { return 3; }

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
                desc->set_current_primitive_set(name, GL_LINE_STRIP);
                desc->move_pos_array(std::move(get_wireframe_coords()));
            }
            return desc;
        }

        std::vector<float> get_mesh_coords() const
        {
            std::vector<float> coords;
            coords.reserve(m_latRes * m_lonRes * 6 * 3);

            for (uint32_t i = 0; i < m_latRes; ++i)
            {
                float phi0 = static_cast<float>(M_PI) * i / m_latRes;
                float phi1 = static_cast<float>(M_PI) * (i + 1) / m_latRes;
                for (uint32_t j = 0; j < m_lonRes; ++j)
                {
                    float th0 = 2.0f * static_cast<float>(M_PI) * j / m_lonRes;
                    float th1 = 2.0f * static_cast<float>(M_PI) * (j + 1) / m_lonRes;

                    auto to_cart = [&](float phi, float theta)
                    {
                        return glm::vec3(
                            m_center.x + m_radius * sin(phi) * cos(theta),
                            m_center.y + m_radius * sin(phi) * sin(theta),
                            m_center.z + m_radius * cos(phi));
                    };

                    glm::vec3 p0 = to_cart(phi0, th0);
                    glm::vec3 p1 = to_cart(phi0, th1);
                    glm::vec3 p2 = to_cart(phi1, th0);
                    glm::vec3 p3 = to_cart(phi1, th1);


                    for (auto &v : {p0, p2, p1})
                    {
                        coords.push_back(v.x);
                        coords.push_back(v.y);
                        coords.push_back(v.z);
                    }

                    for (auto &v : {p1, p2, p3})
                    {
                        coords.push_back(v.x);
                        coords.push_back(v.y);
                        coords.push_back(v.z);
                    }
                }
            }
            return coords;
        }

        std::vector<float> get_wireframe_coords() const
        {
            std::vector<float> coords;

            for (uint32_t i = 1; i < m_latRes; ++i)
            {
                float phi = static_cast<float>(M_PI) * i / m_latRes;
                for (uint32_t j = 0; j <= m_lonRes; ++j)
                {
                    float theta = 2.0f * static_cast<float>(M_PI) * j / m_lonRes;
                    glm::vec3 p(
                        m_center.x + m_radius * sin(phi) * cos(theta),
                        m_center.y + m_radius * sin(phi) * sin(theta),
                        m_center.z + m_radius * cos(phi));
                    coords.push_back(p.x);
                    coords.push_back(p.y);
                    coords.push_back(p.z);
                }
            }
            for (uint32_t j = 0; j < m_lonRes; ++j)
            {
                float theta = 2.0f * static_cast<float>(M_PI) * j / m_lonRes;
                for (uint32_t i = 0; i <= m_latRes; ++i)
                {
                    float phi = static_cast<float>(M_PI) * i / m_latRes;
                    glm::vec3 p(
                        m_center.x + m_radius * sin(phi) * cos(theta),
                        m_center.y + m_radius * sin(phi) * sin(theta),
                        m_center.z + m_radius * cos(phi));
                    coords.push_back(p.x);
                    coords.push_back(p.y);
                    coords.push_back(p.z);
                }
            }
            return coords;
        }

    private:
        glm::vec3 m_center;
        double    m_radius;
        uint32_t  m_latRes, m_lonRes;
        bool      m_filled;
    };

} // namespace gp_primitives

#endif // __GP_SPHERE_HPP__
