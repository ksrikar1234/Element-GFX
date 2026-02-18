

#include <glm/glm.hpp>

#include "gp_gui_opengl_2_1_vertex_array_object.h"
#include "gp_gui_geometry_descriptor.h"

#include "gp_gui_pixel_utils.h"

#include "graphics_api.hpp"
#include "gp_gui_debug.h"


namespace GridPro_GFX
{    
namespace OpenGL_2_1
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
        is_in_selection_mode = false;
        last_init_id = 0; last_pick_entity_count = 0;
    }

    VertexArrayObject::~VertexArrayObject()
    {

    }

    void VertexArrayObject::set_vertex_attribute(std::vector<float>* position_data = nullptr, std::vector<float>* normal_data = nullptr, std::vector<GLubyte>* color_data = nullptr)
    {

    }   
       
    void VertexArrayObject::set_indices(std::vector<uint32_t>* index_data)
    {

    } 
    
    void VertexArrayObject::generate_unique_color_array()
    {
        uint32_t curr_last_init_id = m_geometry_descriptor->get_color_id_reserve_start();
        GLenum pick_scheme = (*m_geometry_descriptor)->get_pick_scheme_enum();
        
        uint32_t curr_entity_count = 0;
        uint32_t vertices_per_primitive = 1; 

        if(pick_scheme == GL_PICK_BY_PRIMITIVE)
        { 
          curr_entity_count = (*m_geometry_descriptor)->get_num_primitives();
          vertices_per_primitive = (*m_geometry_descriptor)->get_num_vertices_per_primitive();
        } 
        else if(pick_scheme == GL_PICK_BY_VERTEX)
        {
          curr_entity_count = (*m_geometry_descriptor)->get_num_vertices();
          vertices_per_primitive = 1;
        }
        else
        {
          curr_entity_count = 0;
          return;
        }
        
        if(curr_last_init_id == last_init_id && curr_entity_count == last_pick_entity_count && vertices_per_primitive == last_pick_vertices_per_primitve_count)
        {
            return;
        }

        last_init_id = curr_last_init_id;
        last_pick_entity_count = curr_entity_count;
        last_pick_vertices_per_primitve_count = vertices_per_primitive;
        
        GP_TRACE("Init ID : ", last_init_id);
        GP_TRACE("Entity Count : ", last_pick_entity_count);
        
        m_unique_color_array.resize(last_pick_entity_count * last_pick_vertices_per_primitve_count * 3);

        for(uint32_t i = 0; i < last_pick_entity_count; i++)
        {
            uint32_t unique_color_id = i + last_init_id;
            PixelData color = PixelData(unique_color_id);
            for(uint32_t j = 0; j < last_pick_vertices_per_primitve_count; j++)
            {
                m_unique_color_array[(i * last_pick_vertices_per_primitve_count + j) * 3 + 0] = color.r ;
                m_unique_color_array[(i * last_pick_vertices_per_primitve_count + j) * 3 + 1] = color.g ;
                m_unique_color_array[(i * last_pick_vertices_per_primitve_count + j) * 3 + 2] = color.b ;
            }
        }
        if(has_index_data())
        flattened_vertex_array = (*m_geometry_descriptor)->get_flattened_position_array();
    }

    void VertexArrayObject::bind()
    {
        calculate_offsets();
        if(PositionData->size() == 0) 
        {
            std::string err = m_geometry_descriptor->get_current_primitive_set_name() + " Position Data is empty\n";
            throw std::runtime_error(err);
        }
        
        if((*m_geometry_descriptor)->isHavingPositonUpdates())
        {
              for(auto& vertex : (*m_geometry_descriptor)->batch_vertex_updates)
                 perform_micro_vertex_update(vertex.index, vertex.m_position[0], vertex.m_position[1], vertex.m_position[2]);
                
              (*m_geometry_descriptor)->batch_vertex_updates.clear();
        }

        RendererAPI<QGL_2_1>()->glEnableClientState(GL_VERTEX_ARRAY);
        
        if(NormalData->size() > 0)
        RendererAPI<QGL_2_1>()->glEnableClientState(GL_NORMAL_ARRAY);

        if(ColorData->size() > 0 || (m_unique_color_array.size() > 0 && is_in_selection_mode))
        {
         RendererAPI<QGL_2_1>()->glEnableClientState(GL_COLOR_ARRAY);
         RendererAPI<QGL_2_1>()->glShadeModel(GL_SMOOTH);
        } 
        if(is_in_selection_mode && (*m_geometry_descriptor)->get_pick_scheme_enum() == GL_PICK_BY_PRIMITIVE)
        RendererAPI<QGL_2_1>()->glVertexPointer(3, GL_FLOAT, 0, flattened_vertex_array.data());
        else
        RendererAPI<QGL_2_1>()->glVertexPointer(3, GL_FLOAT, 0, PositionData->data());
        
        if(NormalData->size() > 0)
           RendererAPI<QGL_2_1>()->glNormalPointer(GL_FLOAT, 0, NormalData->data());
        
        if(m_unique_color_array.size() > 0 && is_in_selection_mode)
          RendererAPI<QGL_2_1>()->glColorPointer(3, GL_UNSIGNED_BYTE, 0, m_unique_color_array.data());

        else if(ColorData->size() > 0)
        {
          RendererAPI<QGL_2_1>()->glColorPointer(3, GL_UNSIGNED_BYTE, 0, ColorData->data()); 
        }
    }

    void VertexArrayObject::unbind()
    {
        is_in_selection_mode = false;
        RendererAPI<QGL_2_1>()->glDisableClientState(GL_VERTEX_ARRAY);
        RendererAPI<QGL_2_1>()->glDisableClientState(GL_NORMAL_ARRAY);
        RendererAPI<QGL_2_1>()->glDisableClientState(GL_COLOR_ARRAY);
    }
    
    void VertexArrayObject::calculate_offsets()
    {
        PositionData = (*m_geometry_descriptor)->get_position_weak_ptr().lock().get();
        NormalData   = (*m_geometry_descriptor)->get_normals_weak_ptr().lock().get();
        ColorData    = (*m_geometry_descriptor)->get_colors_weak_ptr().lock().get();
        IndexData    = (*m_geometry_descriptor)->get_indices_weak_ptr().lock().get();  
    }

    void VertexArrayObject::create_vbo()
    {}
  
    void VertexArrayObject::create_ibo()
    {}

    void VertexArrayObject::update_vertex_attributes(std::vector<float>* position_data, std::vector<float>* normal_data, std::vector<GLubyte>* color_data) 
    {}

    void VertexArrayObject::update_indices(std::vector<uint32_t>* IndexData)
    {}
        
    void VertexArrayObject::perform_micro_vertex_update(const uint32_t& vertex_id, const float& pos_x, const float& pos_y, const float& pos_z)
    {}

    void VertexArrayObject::delete_vbo()
    {}

    void VertexArrayObject::delete_ibo()
    {}

    void VertexArrayObject::delete_vao()
    {}
}

} // namespace GridPro_GFX