#ifndef _GP_OSG_GEOMETRY_HPP
#define _GP_OSG_GEOMETRY_HPP

#include <osg/ref_ptr>
#include <osg/Geometry>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Vec3ub>
#include <osg/Vec4ub>
#include <osg/PrimitiveSet>
#include <osg/Array>

#include <osgFX/Scribe>

#include <osg/Material>
#include <osg/StateSet>

#include "gp_gui_geometry_descriptor.h"

#include <memory>

using namespace gridpro_gui;
namespace GP_OSG
{
    class Geometry
    {
      public:
      Geometry(std::shared_ptr<GeometryDescriptor>& descriptor) : m_descriptor(descriptor)
      {
        m_geode = new osg::Geode();
        m_geometry = new osg::Geometry();
        m_scribe = new osgFX::Scribe();
        m_geode->addDrawable(m_geometry);
        generate();
        use_scribe = (*m_descriptor)->get_wireframe_mode_enum() == GL_WIREFRAME_OVERLAY;
      }
      
      osg::ref_ptr<osg::Node> get_geometry() 
      { 
        if(use_scribe)
          return m_scribe;
        else
          return m_geode;
      }
    
      private:

      void generate()
      {
        // Set the vertices
        if((*m_descriptor)->positions_vector().size())
        {
          printf("Setting vertices\n");  
          osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array();
          const std::vector<float>& pos_vec = (*m_descriptor)->positions_vector();
          vertices->resize(pos_vec.size() / 3);
          std::memcpy(&vertices->front(), pos_vec.data(), pos_vec.size() * sizeof(float));
          m_geometry->setVertexArray(vertices);
        }
       
        if((*m_descriptor)->normals_vector().size())
        {    
          osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array();
          normals->resize((*m_descriptor)->normals_vector().size() / 3);
          const std::vector<float>& norm_vec = (*m_descriptor)->normals_vector();
          std::memcpy(&normals->front(), norm_vec.data(), norm_vec.size() * sizeof(float));
          
          if((*m_descriptor)->normals_vector().size()/3 == (*m_descriptor)->get_num_vertices())
            m_geometry->setNormalArray(normals, osg::Array::BIND_PER_VERTEX);
          else
          m_geometry->setNormalArray(normals, osg::Array::BIND_PER_PRIMITIVE_SET);
          m_geometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);
        }

        if((*m_descriptor)->colors_vector().size())
        {
            osg::ref_ptr<osg::Vec3ubArray> colors = new osg::Vec3ubArray();
            colors->resize((*m_descriptor)->colors_vector().size() / 3);
            const std::vector<uint8_t>& col_vec = (*m_descriptor)->colors_vector();
            std::memcpy(&colors->front(), col_vec.data(), col_vec.size() * sizeof(uint8_t));
        
            // Set the color array and use BIND_PER_VERTEX
            if((*m_descriptor)->colors_vector().size()/3 == (*m_descriptor)->get_num_vertices())
              m_geometry->setColorArray(colors, osg::Array::BIND_PER_VERTEX);
            else
              m_geometry->setColorArray(colors, osg::Array::BIND_PER_PRIMITIVE_SET);
        }
        
        // Set the indices
        if((*m_descriptor)->indices_vector().size())
        {
          GLenum primitive_type = (*m_descriptor)->get_primitive_type_enum();  
          osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(primitive_type);
          const std::vector<unsigned int>& ind_vec = (*m_descriptor)->indices_vector();
          indices->insert(indices->end(), ind_vec.begin(), ind_vec.end());
          m_geometry->addPrimitiveSet(indices);
        }
        else
        {
          // Set the primitive type
          GLenum primitive_type = (*m_descriptor)->get_primitive_type_enum();
          m_geometry->addPrimitiveSet(new osg::DrawArrays(primitive_type, 0, (*m_descriptor)->get_num_unique_positions()));
        }
        configure_material();
      }
      
      void configure_material()
      {
        osg::ref_ptr<osg::Material> material = new osg::Material();
        material->setColorMode(osg::Material::AMBIENT_AND_DIFFUSE);

        material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0.1, 0.0, 0.0, 1.0));
        material->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.1, 0.5, 0.2, 1.0));
        material->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0.3, 0.0, 0.0, 1.0));
        m_geometry->getOrCreateStateSet()->setAttribute(material);
        
        if((*m_descriptor)->get_wireframe_mode_enum() == GL_WIREFRAME_OVERLAY)
        {
         // Wrap geometry with osgFX::Scribe
         m_scribe->addChild(m_geometry);
         auto w_c =(*m_descriptor)->wireframecolor.get_color();
         float w_l = (*m_descriptor)->get_line_width();
         m_scribe->setWireframeColor(osg::Vec4(w_c[0], w_c[1], w_c[2], w_c[3]));
         m_scribe->setWireframeLineWidth(w_l);
        }
      }

      private:
      bool use_scribe = false;
      std::shared_ptr<GeometryDescriptor>& m_descriptor;
      osg::ref_ptr<osg::Geometry> m_geometry;
      osg::ref_ptr<osg::Geode> m_geode;
      osg::ref_ptr<osgFX::Scribe> m_scribe;

    };
}




#endif // _GP_OSG_GEOMETRY_HPP