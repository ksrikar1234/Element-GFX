#ifndef GP_ABSTRACT_VERTEX_ARRAY_OBJECT_H
#define GP_ABSTRACT_VERTEX_ARRAY_OBJECT_H

#include <cstdint>
#include <vector>
#include "gp_gui_typedefs.h"

namespace gridpro_gpu_metrics
{
  static uint32_t gpu_current_vertex_array_size = 0;
}

namespace GridPro_GFX
{
    class GeometryDescriptor;

    class Abstract_VertexArrayObject 
    {
       public :
       
       explicit Abstract_VertexArrayObject(GeometryDescriptor* geometry_descriptor)
       {
              m_geometry_descriptor = geometry_descriptor;
              PositionData = nullptr;
              NormalData   = nullptr;
              ColorData    = nullptr;
              IndexData    = nullptr;
              m_vao = (0);
              m_vbo = (0);
              m_ibo = (0);
              m_vbo_curr_size = (0);
              m_ibo_curr_size = (0);
       }

       virtual ~Abstract_VertexArrayObject() = default;
       
       virtual void update_vertex_attributes(std::vector<float>* position_data, std::vector<float>* normal_data, std::vector<GLubyte>* color_data) = 0;
       virtual void update_indices(std::vector<uint32_t>* index_data) = 0; 

       virtual void bind() = 0;
       virtual void unbind() = 0;
       
       /// @brief Calculate the offsets for the vertex attributes
       virtual void calculate_offsets() = 0;

       /// @brief Create the vertex buffer object
       virtual void create_vbo() = 0;
       virtual void create_ibo() = 0;
       
       virtual void perform_micro_vertex_update(const uint32_t& vertex_id, const float& pos_x, const float& pos_y, const float& pos_z) = 0;       

       virtual void delete_vbo() = 0;
       virtual void delete_ibo() = 0;
       virtual void delete_vao() = 0;
       
       /// @brief   Get the vertex array object size in bytes
       /// @return  The size of the vertex array object in bytes
       uint32_t get_vbo_size() const    { return m_vbo_curr_size; }

       /// @brief Get which vertex attribute data is present
       bool has_normal_attrib() const { return (NormalData != nullptr && NormalData->size()) ? 1 : 0; }
       bool has_color_attrib()  const { return (ColorData  != nullptr && ColorData->size())  ? 1 : 0; }
       
       /// @brief Get if element array buffer is present
       bool has_index_data() const    { return (IndexData != nullptr  && IndexData->size())  ? 1 : 0; }

       protected :
       uint32_t m_vao, m_vbo, m_ibo;
       
       uint32_t vSize, nSize, cSize;
            
       uint32_t vOffset, nOffset, cOffset;

       uint32_t m_vbo_curr_size, m_ibo_curr_size;
        
       GeometryDescriptor* m_geometry_descriptor;

       std::vector<float>*    PositionData;
       std::vector<float>*    NormalData;
       std::vector<GLubyte>*  ColorData;
       std::vector<uint32_t>* IndexData;

       std::vector<GLfloat>  DummyData1;
       std::vector<GLubyte>  DummyData2;
       std::vector<uint32_t> DummyData3;

    };

}    

#endif // GP_GUI_VERTEX_ARRAY_OBJECT_H