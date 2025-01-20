
#ifndef _GP_GUI_CAMERA_H_
#define _GP_GUI_CAMERA_H_

#include <memory>
#include <array>
#include <limits>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include "graphics_api.hpp"
#include <cmath>
#include <mutex>

namespace gridpro_gui
{

    struct bounding_box
    {
        glm::vec3 min, max;

        std::array<glm::vec3, 8> get_bounding_box_nodes() const
        {
            return 
            {
                glm::vec3(min.x, min.y, min.z),
                glm::vec3(min.x, min.y, max.z),
                glm::vec3(min.x, max.y, min.z),
                glm::vec3(min.x, max.y, max.z),
                glm::vec3(max.x, min.y, min.z),
                glm::vec3(max.x, min.y, max.z),
                glm::vec3(max.x, max.y, min.z),
                glm::vec3(max.x, max.y, max.z)     
            };
        }

        glm::vec3 get_updated_camera_position(const glm::vec3 &camera_orientation) const
        {
            glm::vec3 m_center = center();
            float size = glm::dot((max - min), camera_orientation);
            return m_center + (camera_orientation * size/2.0f);
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
    };

    struct view_volume
    {
        float left = -100.0f;
        float right = 100.0f;
        float bottom = -100.0f;
        float top = 100.0f;
        float nearPlane = -400.0f;
        float farPlane = 400.0f;

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
        }
         
        void set_center_of_rotation(const float &x, const float &y, const float &z)
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

        void set_updated_bounding_box(const bounding_box &bb)
        {
            m_original_bounding_box = bb;
            m_bounding_box = bb;
            camera_position = m_bounding_box.get_updated_camera_position(camera_orientation);
            set_updated_view_volume();
        }

        void set_camera_orientation(const glm::vec3 &orientation)
        {
            camera_orientation = orientation;
            ortho_direction = glm::normalize(glm::cross(camera_orientation, camera_updirection));
            set_updated_view_volume();
        }

        void set_camera_updirection(const glm::vec3 &updirection)
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
            GLint view_port[4];
            RendererAPI()->glGetIntegerv(GL_VIEWPORT, view_port);
           
            float del_x = xoffset * (m_view_volume.right - m_view_volume.left) / view_port[2];
            float del_y = yoffset * (m_view_volume.top - m_view_volume.bottom) / view_port[3];

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

            original_cor_screen_coords = world_to_screen(curr_center_of_rotation);
        }

        void processMousePanForCOR(float xoffset, float yoffset)
        {
            GLint view_port[4];
            RendererAPI()->glGetIntegerv(GL_VIEWPORT, view_port);
           
            float del_x = xoffset * (m_view_volume.right - m_view_volume.left) / view_port[2];
            float del_y = yoffset * (m_view_volume.top - m_view_volume.bottom) / view_port[3];
            
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
            std::lock_guard<std::mutex> lock(m_mutex);
            GLint view_port[4];
            RendererAPI()->glGetIntegerv(GL_VIEWPORT, view_port);

            float del_x = deltaX * M_PI / view_port[2];
            float del_y = deltaY * M_PI / view_port[3];

            theta = del_x*3; // pitch
            phi = del_y*3;   // yaw

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
            camera_position  = glm::vec3(rotation_matrix * glm::vec4(camera_position, 1.0f));
            camera_position += curr_center_of_rotation;
            
            camera_orientation = glm::normalize(camera_orientation);
            ortho_direction = glm::normalize(glm::cross(camera_orientation, camera_updirection));
        
            float old_volume = m_view_volume.volume();

            set_updated_view_volume();
            
            //Need to Fix this Jitter
            if(use_custom_center)
            {
                

                glm::vec2 new_screen_coords = world_to_screen(curr_center_of_rotation); 
                
                float xoffset = new_screen_coords.x - original_cor_screen_coords.x;
                float yoffset = new_screen_coords.y - original_cor_screen_coords.y;
                
                processMousePanForCOR(xoffset, yoffset);
            
                //  ALternative method is to adjust the bounding box by translating it by  (curr_center_of_rotation)
                
                // glm::vec3 translation_vector = get_world_space_translation_vector(original_cor_screen_coords, new_screen_coords);

                // glm::vec3 rotated_vector = glm::vec3(rotation_matrix * glm::vec4(translation_vector, 1.0f));

                // translation_vector = rotated_vector;

                // m_bounding_box.min += translation_vector;
                // m_bounding_box.max += translation_vector;
                
                // float old_volume = m_view_volume.volume();
                // set_updated_bounding_box(m_bounding_box);
                // float new_volume = m_view_volume.volume();

                glm::vec2 re_adjusted_screen_coords = world_to_screen(curr_center_of_rotation);

                if(re_adjusted_screen_coords.x != new_screen_coords.x || re_adjusted_screen_coords.y != new_screen_coords.y)
                {
                    // Fix the Error
                    glm::vec2 re_adjusted_screen_coords = world_to_screen(curr_center_of_rotation);
                    float xoffset = re_adjusted_screen_coords.x - original_cor_screen_coords.x;
                    float yoffset = re_adjusted_screen_coords.y - original_cor_screen_coords.y;
                    processMousePanForCOR(xoffset, yoffset);
                }
            }

            float new_volume = m_view_volume.volume();
            
            float ratio = (old_volume/new_volume);

            m_view_volume = m_view_volume * (ratio + 0.5) ;

            set_updated_view_volume();

        }

        void processMouseScroll(const float& yoffset)
        {
            // float ratio = 1 - (0.15 * yoffset);
            // // std::cout << "*** Ratio = " << ratio << "\n";

            // std::array<glm::vec3, 8> new_nodes = m_bounding_box.get_bounding_box_nodes();
            // glm::vec3 center = m_bounding_box.center();
            // for (auto &node : new_nodes)
            // {
            //     node -= center;

            //     float component_along_cam_dir = glm::dot(node, camera_orientation);
            //     node -= camera_orientation * component_along_cam_dir;

            //     node *= (ratio);

            //     node += (camera_orientation * component_along_cam_dir);

            //     node += center;
            // }

            //  m_bounding_box.min = new_nodes[0];
            //  m_bounding_box.max = new_nodes[7];

            // set_updated_bounding_box(m_bounding_box);

            float zoom_ratio = 1 - (0.15 * yoffset);

            m_zoom_ratio *= zoom_ratio;
            
            // glm::vec3 new_view_volume_center = m_view_volume.center();

            // // Calculate the current distance from the camera to the center of the view volume
            // glm::vec3 camera_to_center = camera_position - m_view_volume.center();
            // float distance = glm::length(camera_to_center);

            // // Apply the zoom ratio to the distance
            // distance *= zoom_ratio;

            // // Calculate the new camera position
            // glm::vec3 direction = glm::normalize(camera_to_center);
            // camera_position = new_view_volume_center + direction * distance;

            // m_view_volume = m_view_volume * zoom_ratio;
            // Update the view volume by the zoom ratio
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
            light_position -=  m_view_volume.get_depth() * 1000.0f * camera_orientation;
            return light_position;
        }

        glm::vec3 scrn_to_world(const glm::vec2 &screen_coords , const glm::vec2 &screen_size) const
        {
            // Step 1: Normalize screen coordinates
            glm::vec3 normalized_screen_coords;
            normalized_screen_coords.x = (2.0f * screen_coords.x) / screen_size.x - 1.0f;
            normalized_screen_coords.y = 1.0f - (2.0f * screen_coords.y) / screen_size.y;
            normalized_screen_coords.z = 0.0f; // Assuming you want the coordinates at the near plane

            // Step 2: Create the inverse projection-view matrix
            glm::mat4 inverse_projection_view_matrix = glm::inverse(m_projection * m_view);

            // Step 3: Unproject the normalized screen coordinates
            glm::vec4 world_coords = inverse_projection_view_matrix * glm::vec4(normalized_screen_coords, 1.0f);

            return glm::vec3(world_coords) / world_coords.w;
        }
        
        glm::vec3 screen_to_world(const glm::vec2 &screen_coords) const
        {
            // Step 1: Normalize screen coordinates
            GLint viewport[4];
            RendererAPI()->glGetIntegerv(GL_VIEWPORT, viewport);
            glm::vec2 screen_size = glm::vec2(viewport[2], viewport[3]);
            return scrn_to_world(screen_coords, screen_size);
        }

        glm::vec3 get_world_space_translation_vector(const glm::vec2 &initial_screen_coords, const glm::vec2 &final_screen_coords) const
        {
            glm::vec3 initial_world_coords = screen_to_world(initial_screen_coords);
            glm::vec3 final_world_coords   = screen_to_world(final_screen_coords);
            return final_world_coords - initial_world_coords;
        }

        glm::vec2 world_to_screen(const glm::vec3 &world_coords) const
        {
            glm::vec4 clip_coords = m_projection * m_view * glm::vec4(world_coords, 1.0f);
            clip_coords /= clip_coords.w;

            GLint viewport[4];
            RendererAPI()->glGetIntegerv(GL_VIEWPORT, viewport);

            glm::vec2 screen_coords;
            screen_coords.x = (clip_coords.x + 1.0f) / 2.0f * viewport[2];
            screen_coords.y = (1.0f - clip_coords.y) / 2.0f * viewport[3];

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
            
            camera_position -=  1000.0f* m_zoom_ratio * camera_orientation;
            
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
        
    private:
        void set_updated_view_volume()
        {
            std::array<glm::vec3, 8> nodes = m_bounding_box.get_bounding_box_nodes();
            glm::vec3 center = m_bounding_box.center();

            m_view_volume.left   = std::numeric_limits<float>::max();
            m_view_volume.right  = std::numeric_limits<float>::min();
            m_view_volume.bottom = std::numeric_limits<float>::max();
            m_view_volume.top    = std::numeric_limits<float>::min();
            m_view_volume.nearPlane = std::numeric_limits<float>::max();
            m_view_volume.farPlane  = std::numeric_limits<float>::min();

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
            GLint viewport[4];
            RendererAPI()->glGetIntegerv(GL_VIEWPORT, viewport);
            float aspect_ratio = static_cast<float>(viewport[2]) / static_cast<float>(viewport[3]);

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
            m_view_volume = m_view_volume * 4.0f;
            m_view_volume.nearPlane *= 400.0f;
            m_view_volume.farPlane *= 400.0f;

            m_projection = glm::ortho(m_view_volume.left, m_view_volume.right, m_view_volume.bottom, m_view_volume.top, m_view_volume.nearPlane, m_view_volume.farPlane);

            camera_orientation = glm::normalize(camera_orientation);
            camera_updirection = glm::normalize(camera_updirection);

            m_view = glm::lookAt(camera_position, camera_position + camera_orientation, camera_updirection);

            // std::cout << "View Volume : " << m_view_volume.left << ", " << m_view_volume.right << ", " << m_view_volume.bottom << ", " << m_view_volume.top << ", " << m_view_volume.nearPlane << ", " << m_view_volume.farPlane << std::endl;
            // std::cout << "\nCamera Position: " << glm::to_string(camera_position) << std::endl;
            // std::cout << "\nCamera Orientation: " << glm::to_string(camera_orientation) << std::endl;
            // std::cout << "\nCamera Up Direction: " << glm::to_string(camera_updirection) << std::endl;
        }

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

        std::mutex m_mutex;
    };

} // namespace gridpro_gui

#endif
