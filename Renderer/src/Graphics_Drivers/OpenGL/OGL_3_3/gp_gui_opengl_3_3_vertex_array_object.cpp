#include <glm/glm.hpp>

#include "gp_gui_opengl_3_3_vertex_array_object.h"
#include "gp_gui_geometry_descriptor.h"

#include "graphics_api.hpp"
#include "gp_gui_debug.h"

namespace GridPro_GFX
{    
namespace OpenGL_3_3
{
    VertexArrayObject::VertexArrayObject(GeometryDescriptor* geometry_descriptor) : Abstract_VertexArrayObject(geometry_descriptor)
    {
        PositionData = (*m_geometry_descriptor)->get_position_weak_ptr().lock().get();
        NormalData   = (*m_geometry_descriptor)->get_normals_weak_ptr().lock().get();
        ColorData    = (*m_geometry_descriptor)->get_colors_weak_ptr().lock().get();
        IndexData    = (*m_geometry_descriptor)->get_indices_weak_ptr().lock().get();
        
        if(PositionData->size() == 0) 
        {
            std::string err = m_geometry_descriptor->get_current_primitive_set_name() + " Position Data is empty\n";
            throw std::runtime_error(err);
        }

        calculate_offsets();
        create_vbo();

        if(IndexData->size() != 0)
        {
            create_ibo();
        }
    }

    VertexArrayObject::~VertexArrayObject()
    {
        gridpro_gpu_metrics::gpu_current_vertex_array_size -= get_vbo_size();
        GP_TRACE("Deleting Vertex Array Size = ", gridpro_gpu_metrics::gpu_current_vertex_array_size , "bytes");
        RendererAPI<QGL_3_3>()->glDeleteVertexArrays(1, &m_vao);
        delete_vbo();
        delete_ibo();
    }

    void VertexArrayObject::set_vertex_attribute(std::vector<float>* position_data = nullptr, std::vector<float>* normal_data = nullptr, std::vector<GLubyte>* color_data = nullptr)
    {
        if(position_data != nullptr)  
           PositionData = position_data;
        
        if(normal_data != nullptr)  
           NormalData   = normal_data;

        if(color_data != nullptr)
           ColorData    = color_data;

        try 
        {
            delete_vbo();
        }

        catch(const std::exception& e)
        {
            GP_TRACE(e.what());
        } 

        calculate_offsets();
        create_vbo();
    }   
       
    void VertexArrayObject::set_indices(std::vector<uint32_t>* index_data)
    {
        if(index_data == nullptr) return;
        IndexData = index_data;
        try 
        {
            delete_ibo();
        }

        catch(const std::exception& e)
        {
            GP_TRACE(e.what());
        } 

        create_ibo();
    } 


    void VertexArrayObject::bind()
    {
        if((*m_geometry_descriptor)->isHavingPositonUpdates())
        {
              for(auto& vertex : (*m_geometry_descriptor)->batch_vertex_updates)
                 perform_micro_vertex_update(vertex.index, vertex.m_position[0], vertex.m_position[1], vertex.m_position[2]);
                
              (*m_geometry_descriptor)->batch_vertex_updates.clear();
        }

        if(RendererAPI<QGL_3_3>()->glIsVertexArray(m_vao) != GL_TRUE) 
        { 
            GP_TRACE("VAO is not created : ", (*m_geometry_descriptor)->get_instance_name());
            return;
        }   
        else
        {
            RendererAPI<QGL_3_3>()->glBindVertexArray(m_vao);
            GP_TRACE("VAO is bound : ", (*m_geometry_descriptor)->get_instance_name());
        }
        
        if(RendererAPI<QGL_3_3>()->glIsBuffer(m_ibo) == GL_TRUE)
        {
            RendererAPI<QGL_3_3>()->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);       
        }
    }

    void VertexArrayObject::unbind()
    {
        RendererAPI<QGL_3_3>()->glBindVertexArray(0);
        RendererAPI<QGL_3_3>()->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); 
    }
    
    void VertexArrayObject::calculate_offsets()
    {
        vSize = 0;
        nSize = 0;
        cSize = 0;

        if (PositionData)
            vSize = PositionData->size() * sizeof(float);

        if (NormalData)
            nSize = NormalData->size()   * sizeof(float);

        if (ColorData)
            cSize = ColorData->size()    * sizeof(GLubyte);

        vOffset = 0;
        nOffset = vSize;
        cOffset = vSize + nSize;
        GP_TRACE("VBO info : ", "vSize = ", vSize, " nSize = ", nSize, " cSize = ", cSize, "vOffset = ", vOffset, " nOffset = ", nOffset, " cOffset = ", cOffset);
    }

    void VertexArrayObject::create_vbo()
    {   
        GLsizei stride = 0;
        calculate_offsets();
        /// @brief Allocate the vertex buffer object only if the vertex data size has changed
        /// @note  This is to avoid the reallocation of the VBO for every frame
        if(m_vbo_curr_size != vSize + nSize + cSize)
        { 
           delete_vao();
           delete_vbo();
           RendererAPI<QGL_3_3>()->glGenVertexArrays(1, &m_vao);
           RendererAPI<QGL_3_3>()->glGenBuffers(1, &m_vbo);
           RendererAPI<QGL_3_3>()->glBindVertexArray(m_vao);
           RendererAPI<QGL_3_3>()->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
           RendererAPI<QGL_3_3>()->glBufferData(GL_ARRAY_BUFFER, vSize + nSize + cSize, nullptr, GL_STATIC_DRAW);
           m_vbo_curr_size = vSize + nSize + cSize;
           gridpro_gpu_metrics::gpu_current_vertex_array_size += get_vbo_size();
           GP_TRACE("Adding Vertex Array Size = ", gridpro_gpu_metrics::gpu_current_vertex_array_size, "bytes");
        }
        else
        {
           RendererAPI<QGL_3_3>()->glBindVertexArray(m_vao);
           RendererAPI<QGL_3_3>()->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        }

    
        uint32_t it = 0;

        // Copy data to VBO
        if (vSize != 0)
            RendererAPI<QGL_3_3>()->glBufferSubData(GL_ARRAY_BUFFER, vOffset, vSize, PositionData->data());

        if (nSize != 0)
            RendererAPI<QGL_3_3>()->glBufferSubData(GL_ARRAY_BUFFER, nOffset, nSize, NormalData->data());

        if (cSize != 0)
            RendererAPI<QGL_3_3>()->glBufferSubData(GL_ARRAY_BUFFER, cOffset, cSize, ColorData->data());

        // Set vertex attributes pointers
        if (vSize)
        {
            RendererAPI<QGL_3_3>()->glVertexAttribPointer(it, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
            RendererAPI<QGL_3_3>()->glEnableVertexAttribArray(it);
            ++it;
        }
        
        if (nSize)
        {
            RendererAPI<QGL_3_3>()->glVertexAttribPointer(it, 3, GL_FLOAT, GL_FALSE, 0, (void*)nOffset);
            RendererAPI<QGL_3_3>()->glEnableVertexAttribArray(it);
            ++it;
        }

        if (cSize)
        {
            RendererAPI<QGL_3_3>()->glVertexAttribPointer(it, 3, GL_UNSIGNED_BYTE, GL_FALSE,  0 , (void*)cOffset);
            RendererAPI<QGL_3_3>()->glEnableVertexAttribArray(it);
        }

        // Unbind VBO
        RendererAPI<QGL_3_3>()->glBindBuffer(GL_ARRAY_BUFFER, 0);
        unbind();

        }

      
      void VertexArrayObject::create_ibo()
      {   
          if(m_ibo_curr_size != IndexData->size()) 
          {
            delete_ibo();
            RendererAPI<QGL_3_3>()->glGenBuffers(1, &m_ibo); 
          }

          bind();
          RendererAPI<QGL_3_3>()->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
          RendererAPI<QGL_3_3>()->glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexData->size() * sizeof(uint32_t), IndexData->data(), GL_STATIC_DRAW);
          m_ibo_curr_size = IndexData->size();
          RendererAPI<QGL_3_3>()->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
          unbind();
      }


      void VertexArrayObject::update_vertex_attributes(std::vector<float>* position_data, std::vector<float>* normal_data, std::vector<GLubyte>* color_data) 
      {
            RendererAPI<QGL_3_3>()->glBindVertexArray(m_vao);
            RendererAPI<QGL_3_3>()->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
            calculate_offsets();
            RendererAPI<QGL_3_3>()->glBufferData(GL_ARRAY_BUFFER, vSize + nSize + cSize, nullptr, GL_STATIC_DRAW);
    
            if (vSize)
                RendererAPI<QGL_3_3>()->glBufferSubData(GL_ARRAY_BUFFER, 0, vSize, PositionData->data());
    
            if (nSize)
                RendererAPI<QGL_3_3>()->glBufferSubData(GL_ARRAY_BUFFER, vSize, nSize, NormalData->data());
    
            if (cSize)
                RendererAPI<QGL_3_3>()->glBufferSubData(GL_ARRAY_BUFFER, vSize + nSize, cSize, ColorData->data());
    
            RendererAPI<QGL_3_3>()->glBindBuffer(GL_ARRAY_BUFFER, 0);
            RendererAPI<QGL_3_3>()->glBindVertexArray(0);
      }

        void VertexArrayObject::update_indices(std::vector<uint32_t>* IndexData)
        {
            if(IndexData->size() == 0)
                return;

            RendererAPI<QGL_3_3>()->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
            RendererAPI<QGL_3_3>()->glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexData->size() * sizeof(uint32_t), IndexData->data(), GL_STATIC_DRAW);
            RendererAPI<QGL_3_3>()->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
        
        void VertexArrayObject::perform_micro_vertex_update(const uint32_t& vertex_id, const float& pos_x, const float& pos_y, const float& pos_z)
        {
            glm::vec3 new_position(pos_x, pos_y, pos_z);
            RendererAPI<QGL_3_3>()->glBindVertexArray(m_vao);
            RendererAPI<QGL_3_3>()->glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
            RendererAPI<QGL_3_3>()->glBufferSubData(GL_ARRAY_BUFFER, vertex_id * sizeof(glm::vec3), sizeof(glm::vec3), &new_position.x);
            RendererAPI<QGL_3_3>()->glBindBuffer(GL_ARRAY_BUFFER, 0);
            unbind();
        }

        void VertexArrayObject::delete_vbo()
        {
            if(RendererAPI<QGL_3_3>()->glIsBuffer(m_vbo) == GL_TRUE)
            {
               RendererAPI<QGL_3_3>()->glDeleteBuffers(1, &m_vbo);
               m_vbo = 0;
            }
            else
               GP_TRACE("VBO is already deleted");   
        }

        void VertexArrayObject::delete_ibo()
        {
            if(RendererAPI<QGL_3_3>()->glIsBuffer(m_ibo) == GL_TRUE)
            {
               RendererAPI<QGL_3_3>()->glDeleteBuffers(1, &m_ibo);
               m_ibo = 0;
            }
            else
              GP_TRACE("IBO is already deleted");  
        }

        void VertexArrayObject::delete_vao()
        {
            bind(); 
            if(RendererAPI<QGL_3_3>()->glIsVertexArray(m_vao) == GL_TRUE)
            {
               RendererAPI<QGL_3_3>()->glDeleteVertexArrays(1, &m_vao);
               m_vao = 0;
            }
            unbind(); 
        }
}

} // namespace GridPro_GFX