#ifndef GP_GUI_OPENGL_3_3_VERTEX_ARRAY_OBJECT_H
#define GP_GUI_OPENGL_3_3_VERTEX_ARRAY_OBJECT_H

#include "abstract_vertex_array_object.hpp"

namespace gridpro_gui
{

namespace OpenGL_3_3
{

    class VertexArrayObject : public Abstract_VertexArrayObject
    {
       public :
       explicit VertexArrayObject(GeometryDescriptor* geometry_descriptor);
       virtual ~VertexArrayObject() override;
       
       void set_vertex_attribute(std::vector<float>* position_data, std::vector<float>* normal_data, std::vector<GLubyte>* color_data);
       void set_indices(std::vector<uint32_t>* index_data); 

       void bind();
       void unbind();

       void perform_micro_vertex_update(const uint32_t& vertex_id, const float& pos_x, const float& pos_y, const float& pos_z);       

       void update_vertex_attributes(std::vector<float>* position_data, std::vector<float>* normal_data, std::vector<GLubyte>* color_data) ;
       void update_indices(std::vector<uint32_t>* index_data);

       private :
       /// @brief Calculate the offsets for the vertex attributes
       void calculate_offsets();

       /// @brief Create the vertex buffer object
       void create_vbo();
       void create_ibo();
       
       void delete_vbo();
       void delete_ibo();
       void delete_vao();
    };
}
}    

#endif // GP_GUI_OPENGL_3_3_VERTEX_ARRAY_OBJECT_H