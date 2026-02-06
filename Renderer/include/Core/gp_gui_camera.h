
#ifndef _GP_GUI_CAMERA_H_
#define _GP_GUI_CAMERA_H_

#include <array>
#include <limits>
#include <cmath>
#include <fstream>
#include <iomanip>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "graphics_api.hpp"


namespace GridPro_GFX
{

    struct bounding_box
    {
        glm::vec3 min, max;

        std::array<glm::vec3, 8> get_bounding_box_nodes() const
        {
            return {
                glm::vec3(min.x, min.y, min.z),
                glm::vec3(min.x, min.y, max.z),
                glm::vec3(min.x, max.y, min.z),
                glm::vec3(min.x, max.y, max.z),
                glm::vec3(max.x, min.y, min.z),
                glm::vec3(max.x, min.y, max.z),
                glm::vec3(max.x, max.y, min.z),
                glm::vec3(max.x, max.y, max.z)};
        }

        glm::vec3 get_updated_camera_position(const glm::vec3 &camera_orientation) const
        {
            glm::vec3 m_center = center();
            float size = glm::dot((max - min), camera_orientation);
            return m_center + (camera_orientation * size / 2.0f);
        }

        glm::vec3 center() const
        {
            return (min + max) / 2.0f;
        }

        float get_length() const
        {
            return max.x - min.x;
        }

        float get_width() const
        {
            return max.y - min.y;
        }

        float get_height() const
        {
            return max.z - min.z;
        }

        float get_diagonal() const
        {
            return glm::length(max - min);
        }

        float get_min_dimension() const
        {
            return std::min(std::min(get_length(), get_width()), get_height());
        }

        float get_max_dimension() const
        {
            return std::max(std::max(get_length(), get_width()), get_height());
        }

        void swap(bounding_box &bb)
        {
            std::swap(min, bb.min);
            std::swap(max, bb.max);
        }

        void make_cube()
        {
           glm::vec3 center = 0.5f * (min + max);
           glm::vec3 extent = max - min;

           float maxExtent = std::max({ extent.x, extent.y, extent.z });

           glm::vec3 halfSize = glm::vec3(maxExtent * 0.5f);

           min = center - halfSize;
           max = center + halfSize;
        }

    };

    struct view_volume
    {
        float left = -100.0f;
        float right = 100.0f;
        float bottom = -100.0f;
        float top = 100.0f;
        float nearPlane = -10.0f;
        float farPlane = 10.0f;

        glm::vec3 center() const
        {
            return glm::vec3((right + left) / 2.0f, (top + bottom) / 2.0f, (farPlane + nearPlane) / 2.0f);
        }

        // use multiplication operator
        view_volume operator*(float factor)
        {
            view_volume new_view_volume;
            new_view_volume.left = left * factor;
            new_view_volume.right = right * factor;
            new_view_volume.bottom = bottom * factor;
            new_view_volume.top = top * factor;
            return new_view_volume;
        }

        float get_length() const
        {
            return right - left;
        }

        float get_width() const
        {
            return top - bottom;
        }

        float get_depth() const
        {
            return farPlane - nearPlane;
        }

        float volume()
        {
            return get_length() * get_width() * get_depth();
        }

        void set_view_volume(float new_volume)
        {
            float current_volume = volume();
            float ratio = std::cbrt(new_volume / current_volume);
            left *= ratio;
            right *= ratio;
            bottom *= ratio;
            top *= ratio;
            nearPlane *= ratio;
            farPlane *= ratio;
        }
    };

    class Ray
    {
    public:
        Ray(const glm::vec3 &origin, const glm::vec3 &direction)
            : m_origin(origin), m_direction(direction) {}

        glm::vec3 origin() const { return m_origin; }
        glm::vec3 direction() const
        {
            return m_direction;
        }
        glm::vec3 point_at_parameter(float t) const
        { return m_origin + t * m_direction; }

        bool intersect(const std::array<glm::vec3, 2>& line, glm::vec3&intersected_point)
        {
            constexpr float EPSILON = 1e-6f;

            glm::vec3 p0 = line[0];
            glm::vec3 p1 = line[1];
            glm::vec3 segment_direction = p1 - p0;

            glm::vec3 cross_dir = glm::cross(m_direction, segment_direction);
            float det = glm::dot(cross_dir, cross_dir);

            if (det < EPSILON)
                return false; 

            glm::vec3 diff = m_origin - p0;
            glm::vec3 cross_diff = glm::cross(diff, segment_direction);

            float t = glm::dot(cross_diff, cross_dir) / det;
            float v = glm::dot(glm::cross(diff, m_direction), cross_dir) / det;

            if (t < 0.0f || v < 0.0f || v > 1.0f)
                return false; 

            // Compute intersection point
            intersected_point = m_origin + t * m_direction;
            return true;
        }

        // Intersect with a Plane
        bool intersect(double a, double b, double c, double d, glm::vec3 &intersected_point)
        {
            constexpr double EPSILON = 1e-6;

            glm::vec3 normal(a, b, c);
            double denominator = glm::dot(normal, m_direction);

            if (std::abs(denominator) < EPSILON)
                return false; // Ray is parallel to the plane

            double t = -(glm::dot(normal, m_origin) + d) / denominator;

            if (t < 0)
                return false; // Intersection is behind the ray

            intersected_point = m_origin + static_cast<float>(t) * m_direction;
            return true;
        }

        bool intersect(const std::array<glm::vec3, 3> &triangle, glm::vec3 &intersected_point)
        {
            constexpr float EPSILON = 1e-6f;

            glm::vec3 v0 = triangle[0];
            glm::vec3 v1 = triangle[1];
            glm::vec3 v2 = triangle[2];

            glm::vec3 edge1 = v1 - v0;
            glm::vec3 edge2 = v2 - v0;

            glm::vec3 h = glm::cross(m_direction, edge2);
            float a = glm::dot(edge1, h);

            if (std::abs(a) < EPSILON)
                return false; // Parallel to the triangle plane

            float f = 1.0f / a;
            glm::vec3 s = m_origin - v0;
            float u = f * glm::dot(s, h);

            if (u < 0.0f || u > 1.0f)
                return false; // Outside the triangle

            glm::vec3 q = glm::cross(s, edge1);
            float v = f * glm::dot(m_direction, q);

            if (v < 0.0f || (u + v) > 1.0f)
                return false; // Outside the triangle

            float t = f * glm::dot(edge1, q); 

            if (t > EPSILON) // Intersection in front of the ray
            {
                intersected_point = m_origin + t * m_direction;
                return true;
            }

            return false; // Intersection behind the ray origin
        }

        bool intersect(const std::array<glm::vec3, 4>& quad, glm::vec3& intersected_point)
        {
            // // Compute normals for the two triangles forming the quad
            // glm::vec3 normal1 = glm::normalize(glm::cross(quad[1] - quad[0], quad[2] - quad[0]));
            // glm::vec3 normal2 = glm::normalize(glm::cross(quad[2] - quad[0], quad[3] - quad[0]));

            // // Check if the normals are consistent (i.e., they should be nearly the same)
            // if (glm::dot(normal1, normal2) < 0.999f) // Using a threshold for floating-point precision
            //     return false;                        // The quad is not convex or is self-intersecting

            std::array<glm::vec3, 3> tri1 = {quad[0], quad[1], quad[2]};
            std::array<glm::vec3, 3> tri2 = {quad[0], quad[2], quad[3]};

            if (intersect(tri1, intersected_point))
                return true;

            return intersect(tri2, intersected_point);
        }

        glm::vec3 project_point(const glm::vec3& in_point) const
        {
            // ensure direction is unit-length
            glm::vec3 d = glm::normalize(m_direction);

            // vector from ray origin to input point
            glm::vec3 v = in_point - m_origin;

            // distance along the ray
            float t = glm::dot(v, d);

            // clamp to ray so we don't go behind the origin
            if (t < 0.0f) t = 0.0f;

            // return the closest point on the ray
            return m_origin + d * t;
        }

    private:
        glm::vec3 m_origin, m_direction;
    };

    class OrthographicCamera
    {
    public:
        OrthographicCamera(const glm::vec3 &camera_position = glm::vec3(0.0f), const glm::vec3 &camera_orientation = glm::vec3(0.0f, 0.0f, -1.0f), const glm::vec3 &camera_updirection = glm::vec3(0.0f, 1.0f, 0.0f))
            : camera_position(camera_position), camera_orientation(camera_orientation), camera_updirection(camera_updirection)
        {
            ortho_direction = glm::normalize(glm::cross(camera_orientation, camera_updirection));
            sensitivity = 0.5f;
            theta = 0.0f;
            phi = 0.0f;
            m_zoom_ratio = 1.0f;
            use_custom_center = false;
            deserialize_camera();
        }

        ~OrthographicCamera()
        {
            serialize_camera();
        }
        
        void set_screen_dims(glm::vec2 curr_screen_dims)
        {
            m_screen_dims = curr_screen_dims;
        }

        void set_center_of_rotation(const float& x, const float& y, const float& z)
        {
            curr_center_of_rotation = glm::vec3(x, y, z);
            original_cor_screen_coords = world_to_screen(curr_center_of_rotation);
            use_custom_center = true;
        }

        void set_center_of_rotation_to_bounding_box_center()
        {
            curr_center_of_rotation = m_bounding_box.center();
            use_custom_center = false;
        }

        void set_updated_bounding_box(const bounding_box& bb)
        {
            m_bounding_box = bb;
            m_bounding_box.make_cube();
            camera_position = m_bounding_box.get_updated_camera_position(camera_orientation);
            set_updated_view_volume();
        }

        void set_camera_orientation(const glm::vec3& orientation)
        {
            camera_orientation = orientation;
            ortho_direction = glm::normalize(glm::cross(camera_orientation, camera_updirection));
            set_updated_view_volume();
        }

        void set_camera_updirection(const glm::vec3& updirection)
        {
            camera_updirection = updirection;
            ortho_direction = glm::normalize(glm::cross(camera_orientation, camera_updirection));
            set_updated_view_volume();
        }

        void update_projection()
        {
            set_updated_view_volume();
        }

        glm::mat4 GetProjectionMatrix() const
        {
            return m_projection;
        }

        glm::mat4 GetViewMatrix() const
        {
            return m_view;
        }

        void reset_zoom_ratio()
        {
            m_zoom_ratio = 0.5f;
        }

        void snap_to_xy_plane()
        {
            camera_orientation = glm::vec3(0.0f, 0.0f, -1.0f);
            camera_updirection = glm::vec3(0.0f, 1.0f, 0.0f);
            ortho_direction = glm::normalize(glm::cross(camera_orientation, camera_updirection));
            set_updated_view_volume();
        }

        void snap_to_xz_plane()
        {
            camera_orientation = glm::vec3(0.0f, -1.0f, 0.0f);
            camera_updirection = glm::vec3(0.0f, 0.0f, 1.0f);
            ortho_direction = glm::normalize(glm::cross(camera_orientation, camera_updirection));
            set_updated_view_volume();
        }

        void snap_to_yz_plane()
        {
            camera_orientation = glm::vec3(-1.0f, 0.0f, 0.0f);
            camera_updirection = glm::vec3(0.0f, 0.0f, 1.0f);
            ortho_direction = glm::normalize(glm::cross(camera_orientation, camera_updirection));
            set_updated_view_volume();
        }

        void snap_to_yx_plane()
        {
            camera_orientation = glm::vec3(0.0f, 1.0f, 0.0f);
            camera_updirection = glm::vec3(0.0f, 0.0f, -1.0f);
            ortho_direction = glm::normalize(glm::cross(camera_orientation, camera_updirection));
            set_updated_view_volume();
        }

        void snap_to_zx_plane()
        {
            camera_orientation = glm::vec3(0.0f, 0.0f, 1.0f);
            camera_updirection = glm::vec3(0.0f, 1.0f, 0.0f);
            ortho_direction = glm::normalize(glm::cross(camera_orientation, camera_updirection));
            set_updated_view_volume();
        }

        void snap_to_zy_plane()
        {
            camera_orientation = glm::vec3(1.0f, 0.0f, 0.0f);
            camera_updirection = glm::vec3(0.0f, 1.0f, 0.0f);
            ortho_direction = glm::normalize(glm::cross(camera_orientation, camera_updirection));
            set_updated_view_volume();
        }

        void snap_to_isometric_view()
        {
            camera_orientation = glm::vec3(-1.0f, -1.0f, -1.0f);
            camera_updirection = glm::vec3(0.0f, 0.0f, 1.0f);
            ortho_direction = glm::normalize(glm::cross(camera_orientation, camera_updirection));
            set_updated_view_volume();
        }

        void processMousePan(float xoffset, float yoffset)
        {
            float del_x = xoffset * (m_view_volume.right - m_view_volume.left) / m_screen_dims.x;
            float del_y = yoffset * (m_view_volume.top - m_view_volume.bottom) / m_screen_dims.y;

            ortho_direction = glm::normalize(glm::cross(camera_orientation, camera_updirection));

            m_bounding_box.min += ortho_direction * del_x;
            m_bounding_box.max += ortho_direction * del_x;

            m_bounding_box.min -= camera_updirection * del_y;
            m_bounding_box.max -= camera_updirection * del_y;

            if (!use_custom_center)
            {
                curr_center_of_rotation = m_bounding_box.center();
            }

            m_bounding_box.make_cube();
            set_updated_bounding_box(m_bounding_box);

            original_cor_screen_coords = world_to_screen(curr_center_of_rotation);
        }

        void processMousePanForCOR(float xoffset, float yoffset)
        {
            float del_x = xoffset * (m_view_volume.right - m_view_volume.left)  / m_screen_dims.x;
            float del_y = yoffset * (m_view_volume.top - m_view_volume.bottom)  / m_screen_dims.y;

            ortho_direction = glm::normalize(glm::cross(camera_orientation, camera_updirection));

            m_bounding_box.min += ortho_direction * del_x;
            m_bounding_box.max += ortho_direction * del_x;

            m_bounding_box.min -= camera_updirection * del_y;
            m_bounding_box.max -= camera_updirection * del_y;

            if (!use_custom_center)
            {
                curr_center_of_rotation = m_bounding_box.center();
            }

            set_updated_bounding_box(m_bounding_box);
        }

        void processMouseRotation(float deltaX, float deltaY)
        {
            float del_x = deltaX * M_PI / m_screen_dims.x;
            float del_y = deltaY * M_PI / m_screen_dims.y;

            theta = del_x * 3; // pitch
            phi = del_y * 3;   // yaw

            camera_orientation = glm::normalize(camera_orientation);
            camera_updirection = glm::normalize(camera_updirection);

            glm::mat4 rotation_matrix = glm::rotate(glm::mat4(1.0f), theta, camera_updirection);
            rotation_matrix = glm::rotate(rotation_matrix, phi, ortho_direction);

            camera_orientation = glm::vec3(rotation_matrix * glm::vec4(camera_orientation, 1.0f));
            camera_updirection = glm::vec3(rotation_matrix * glm::vec4(camera_updirection, 1.0f));

            camera_orientation = glm::normalize(camera_orientation);
            camera_updirection = glm::normalize(camera_updirection);

            if (!use_custom_center)
            {
                curr_center_of_rotation = m_bounding_box.center();
            }

            camera_position -= curr_center_of_rotation;
            camera_position = glm::vec3(rotation_matrix * glm::vec4(camera_position, 1.0f));
            camera_position += curr_center_of_rotation;

            camera_orientation = glm::normalize(camera_orientation);
            ortho_direction = glm::normalize(glm::cross(camera_orientation, camera_updirection));

            float old_volume = m_view_volume.volume();

            // set_updated_view_volume();

            // Need to Fix this Jitter
            if (use_custom_center)
            {
                glm::vec2 new_screen_coords = world_to_screen(curr_center_of_rotation);

                float xoffset = new_screen_coords.x - original_cor_screen_coords.x;
                float yoffset = new_screen_coords.y - original_cor_screen_coords.y;

                processMousePanForCOR(xoffset, yoffset);

                glm::vec2 re_adjusted_screen_coords = world_to_screen(curr_center_of_rotation);

                if (re_adjusted_screen_coords.x != new_screen_coords.x || re_adjusted_screen_coords.y != new_screen_coords.y)
                {
                    // Fix the Error
                    glm::vec2 re_adjusted_screen_coords = world_to_screen(curr_center_of_rotation);
                    float xoffset = re_adjusted_screen_coords.x - original_cor_screen_coords.x;
                    float yoffset = re_adjusted_screen_coords.y - original_cor_screen_coords.y;
                    processMousePanForCOR(xoffset, yoffset);
                }
            }

            // float new_volume = m_view_volume.volume();
            // float ratio = (old_volume / new_volume);
            // m_view_volume = m_view_volume * (ratio + 0.5);
            // set_updated_view_volume();

            camera_position -= curr_center_of_rotation;
            camera_position = glm::vec3(rotation_matrix * glm::vec4(camera_position, 1.0f));
            camera_position += curr_center_of_rotation;

            camera_orientation = glm::normalize(camera_orientation);
            ortho_direction = glm::normalize(glm::cross(camera_orientation, camera_updirection));

            // REMOVE: The old_volume / new_volume logic and set_updated_view_volume()
            // INSTEAD: Just update the View Matrix
            m_view = glm::lookAt(camera_position, camera_position + camera_orientation, camera_updirection);

            processMousePanForCOR(0, 0);
        }

        void processMouseScroll(const float& yoffset)
        {
            float zoom_ratio = 1 - (0.15 * yoffset);

            m_zoom_ratio *= zoom_ratio;

            float width = (m_view_volume.right - m_view_volume.left) * zoom_ratio;
            float height = (m_view_volume.top - m_view_volume.bottom) * zoom_ratio;
            float centerX = (m_view_volume.left + m_view_volume.right) / 2.0f;
            float centerY = (m_view_volume.bottom + m_view_volume.top) / 2.0f;

            m_view_volume.left = centerX - width / 2.0f;
            m_view_volume.right = centerX + width / 2.0f;
            m_view_volume.bottom = centerY - height / 2.0f;
            m_view_volume.top = centerY + height / 2.0f;

            m_projection = glm::ortho(m_view_volume.left, m_view_volume.right, m_view_volume.bottom, m_view_volume.top, m_view_volume.nearPlane, m_view_volume.farPlane);
            m_view *= zoom_ratio;
        }

        glm::vec3 get_camera_position() const
        {
            return camera_position;
        }

        glm::vec3 get_camera_orientation() const
        {
            return camera_orientation;
        }

        glm::vec3 get_camera_updirection() const
        {
            return camera_updirection;
        }

        glm::vec3 get_current_center_of_rotation() const
        {
            return curr_center_of_rotation;
        }

        glm::vec3 get_bounding_box_center() const
        {
            return m_bounding_box.center();
        }

        bounding_box get_bounding_box() const
        {
            return m_bounding_box;
        }

        glm::vec3 get_view_volume_center() const
        {
            return m_view_volume.center();
        }

        glm::vec3 get_light_position()
        {
            glm::vec3 light_position = camera_position;
            light_position -= m_view_volume.get_depth() * 1000.0f * camera_orientation;
            return light_position;
        }

        Ray generate_mouse_ray(const glm::vec2 &screen_coords) const
        {
            return Ray(screen_to_world(screen_coords), camera_orientation);
        }

        glm::vec3 scrn_to_world(const glm::vec2 &screen_coords, const glm::vec2 &screen_size, float z_buffer_value = 0.0f) const
        {
            // 1) adjust for viewport origin and pixel center
            GLint vp[4];
            RendererAPI()->glGetIntegerv(GL_VIEWPORT, vp);

            float x = (screen_coords.x - vp[0]) + 0.5f;
            float y = (screen_coords.y - vp[1]) + 0.5f;

            // 2) normalize into NDC
            glm::vec3 ndc;
            ndc.x = (2.0f * x) / (screen_size.x - 1.0f) - 1.0f;
            ndc.y = 1.0f - (2.0f * y) / (screen_size.y - 1.0f);
            ndc.z = 2.0f * z_buffer_value - 1.0f;

            // 3) unproject
            glm::mat4 invPV = glm::inverse(m_projection * m_view);
            glm::vec4 wc = invPV * glm::vec4(ndc, 1.0f);
            glm::vec3 world_coord =  glm::vec3(wc) / wc.w;

            return world_coord;
        }

        glm::vec3 screen_to_world(const glm::vec2& screen_coords, float z_buffer_value = 0.0f) const
        {
            // Step 1: Normalize screen coordinates
            return scrn_to_world(screen_coords, m_screen_dims, z_buffer_value);
        }

        glm::vec3 get_world_space_translation_vector(const glm::vec2 &initial_screen_coords, const glm::vec2 &final_screen_coords) const
        {
            glm::vec3 initial_world_coords = screen_to_world(initial_screen_coords);
            glm::vec3 final_world_coords = screen_to_world(final_screen_coords);
            return final_world_coords - initial_world_coords;
        }

        glm::vec2 world_to_screen(const glm::vec3 &world_coords) const
        {
            glm::vec4 clip_coords = m_projection * m_view * glm::vec4(world_coords, 1.0f);
            clip_coords /= clip_coords.w;

            glm::vec2 screen_coords;
            screen_coords.x = (clip_coords.x + 1.0f) / 2.0f * m_screen_dims.x;
            screen_coords.y = (1.0f - clip_coords.y) / 2.0f * m_screen_dims.y;

            return screen_coords;
        }

        bool is_outside_of_screen(const glm::vec3 &world_coords) const
        {
            glm::vec4 clip_coords = m_projection * m_view * glm::vec4(world_coords, 1.0f);
            clip_coords /= clip_coords.w;

            return clip_coords.x < -1.0f || clip_coords.x > 1.0f || clip_coords.y < -1.0f || clip_coords.y > 1.0f || clip_coords.z < -1.0f || clip_coords.z > 1.0f;
        }

        void processMouseScroll2(const float &yoffset)
        {
            float zoom_ratio = 1 - (0.15 * yoffset);

            m_zoom_ratio *= zoom_ratio;

            camera_position -= 1000.0f * m_zoom_ratio * camera_orientation;

            set_updated_view_volume();
        }

        float get_zoom_ratio() const
        {
            return m_zoom_ratio;
        }

        void set_zoom_ratio(const float &zoom_ratio)
        {
            m_zoom_ratio = zoom_ratio;
            set_updated_view_volume();
        }

        void frame_and_zoom_to_point(float mouse_x, float mouse_y, float zoom_factor)
        {
            float width  = static_cast<float>(m_screen_dims.x);
            float height = static_cast<float>(m_screen_dims.y);

            glm::vec2 screenCenter{ width  * 0.5f, height * 0.5f };

            glm::vec2 cursor{ mouse_x,  mouse_y };
            glm::vec2 delta = cursor - screenCenter;

            processMousePan(delta.x, delta.y);
        }

    private:
        void set_updated_view_volume()
        {
            // 1. Get the center and the maximum reach (radius) of the box
            glm::vec3 center = m_bounding_box.center();
            float radius = glm::length(m_bounding_box.max - center); // Distance to furthest corner

            // 2. Set bounds based on radius, not the current projection of nodes
            // This ensures 'left, right, top, bottom' never change during rotation
            m_view_volume.left = -radius;
            m_view_volume.right = radius;
            m_view_volume.bottom = -radius;
            m_view_volume.top = radius;

            // Near/Far can be large to avoid clipping
            m_view_volume.nearPlane = -radius * 100.0f;
            m_view_volume.farPlane = radius * 100.0f;

            // 3. Apply the user's manual zoom ratio
            m_view_volume = m_view_volume * m_zoom_ratio;

            adjust_view_volume_based_on_aspect_ratio();
            calculate_view_projection_matrix();
        }

        void set_updated_view_volume2()
        {
            std::array<glm::vec3, 8> nodes = m_bounding_box.get_bounding_box_nodes();
            glm::vec3 center = m_bounding_box.center();

            m_view_volume.left = std::numeric_limits<float>::max();
            m_view_volume.right = std::numeric_limits<float>::min();
            m_view_volume.bottom = std::numeric_limits<float>::max();
            m_view_volume.top = std::numeric_limits<float>::min();
            m_view_volume.nearPlane = std::numeric_limits<float>::max();
            m_view_volume.farPlane = std::numeric_limits<float>::min();

            for (const auto &node : nodes)
            {
                glm::vec3 node_view_space = node - center;

                m_view_volume.left = std::min(m_view_volume.left, glm::dot(node_view_space, ortho_direction));
                m_view_volume.right = std::max(m_view_volume.right, glm::dot(node_view_space, ortho_direction));
                m_view_volume.bottom = std::min(m_view_volume.bottom, glm::dot(node_view_space, camera_updirection));
                m_view_volume.top = std::max(m_view_volume.top, glm::dot(node_view_space, camera_updirection));
                m_view_volume.nearPlane = std::min(m_view_volume.nearPlane, glm::dot(node_view_space, camera_orientation));
                m_view_volume.farPlane = std::max(m_view_volume.farPlane, glm::dot(node_view_space, camera_orientation));
            }

            m_view_volume = m_view_volume * m_zoom_ratio;

            adjust_view_volume_based_on_aspect_ratio();
            ortho_direction = glm::normalize(glm::cross(camera_orientation, camera_updirection));
            calculate_view_projection_matrix();
        }

        void adjust_view_volume_based_on_aspect_ratio()
        {
            float aspect_ratio = static_cast<float>(m_screen_dims.x) / static_cast<float>(m_screen_dims.y);

            float ortho_distance = m_view_volume.right - m_view_volume.left;
            float up_direction_distance = m_view_volume.top - m_view_volume.bottom;

            float current_view_volume_aspect_ratio = ortho_distance / up_direction_distance;

            if (current_view_volume_aspect_ratio < aspect_ratio)
            {
                float new_ortho_distance = up_direction_distance * aspect_ratio;
                float adjustment = (new_ortho_distance - ortho_distance) / 2.0f;
                m_view_volume.left -= adjustment;
                m_view_volume.right += adjustment;
            }
            else
            {
                float new_up_direction_distance = ortho_distance / aspect_ratio;
                float adjustment = (new_up_direction_distance - up_direction_distance) / 2.0f;
                m_view_volume.bottom -= adjustment;
                m_view_volume.top += adjustment;
            }
        }

        void calculate_view_projection_matrix()
        {
            // m_view_volume = m_view_volume * 10.0f;
            // m_view_volume.nearPlane *= 10.0f;
            // m_view_volume.farPlane *= 10.0f;

            m_view_volume = m_view_volume * 4.0f;
            m_view_volume.nearPlane *= 400.0f;
            m_view_volume.farPlane *= 400.0f;

            m_projection = glm::ortho(m_view_volume.left, m_view_volume.right, m_view_volume.bottom, m_view_volume.top, m_view_volume.nearPlane, m_view_volume.farPlane);

            camera_orientation = glm::normalize(camera_orientation);
            camera_updirection = glm::normalize(camera_updirection);

            m_view = glm::lookAt(camera_position, camera_position + camera_orientation, camera_updirection);
        }

    void serialize_camera(std::string filename = "")
    {
        if (filename.empty())
        {
            const char* env = std::getenv("GP_VIEW_CONFIG");
            filename = env ? env : ".gridpro_view.config";
        }

        std::ofstream ofs(filename);
        if (!ofs)
        {
            std::cerr << "[ERROR] Could not open " << filename << " for writing.\n";
            return;
        }

        ofs << std::fixed << std::setprecision(6);
        ofs << "# GridPro Camera Configuration\n";
        ofs << "camera_position " 
            << camera_position.x << " " << camera_position.y << " " << camera_position.z << "\n";
        ofs << "camera_orientation " 
            << camera_orientation.x << " " << camera_orientation.y << " " << camera_orientation.z << "\n";
        ofs << "camera_updirection " 
            << camera_updirection.x << " " << camera_updirection.y << " " << camera_updirection.z << "\n";
        ofs << "zoom_ratio " << m_zoom_ratio << "\n";

        ofs.close();
        std::cout << "[INFO] Camera configuration saved to " << filename << "\n";
    }

    void deserialize_camera(std::string filename = "")
    {
        if (filename.empty())
        {
            const char* env = std::getenv("GP_VIEW_CONFIG");
            filename = env ? env : ".gridpro_view.config";
        }

        std::ifstream ifs(filename);
        if (!ifs)
        {
            std::cout << "[INFO] No existing camera config found (" << filename << "), using defaults.\n";
            return;
        }

        std::string key;
        while (ifs >> key)
        {
            if (key == "camera_position")
                ifs >> camera_position.x >> camera_position.y >> camera_position.z;
            else if (key == "camera_orientation")
                ifs >> camera_orientation.x >> camera_orientation.y >> camera_orientation.z;
            else if (key == "camera_updirection")
                ifs >> camera_updirection.x >> camera_updirection.y >> camera_updirection.z;
            else if (key == "zoom_ratio")
                ifs >> m_zoom_ratio;
            else
                ifs.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        ifs.close();
        std::cout << "[INFO] Camera configuration loaded from " << filename << "\n";
    }

private:

    private:
        glm::vec3 camera_position;
        glm::vec3 camera_orientation;
        glm::vec3 camera_updirection;
        glm::vec3 ortho_direction;

        // Current view volume
        view_volume m_view_volume;

        // Current bounding box
        bounding_box m_bounding_box;

        // Original bounding box
        bounding_box m_original_bounding_box;

        glm::vec3 curr_center_of_rotation;
        glm::vec2 original_cor_screen_coords;

        bool use_custom_center;

        bool is_rotating;

        glm::mat4 m_projection;
        glm::mat4 m_view;

        float theta;
        float phi;
        float sensitivity;

        float m_zoom_ratio;
        glm::vec2 m_screen_dims;
    };

} // namespace GridPro_GFX

#endif
