#ifndef _GP_GUI_GEOMETRY_DESCRIPTOR_DRAWABLE_H_
#define _GP_GUI_GEOMETRY_DESCRIPTOR_DRAWABLE_H_

/// @author srikar@gridpro.com

/// @file    _GP_GUI_GEOMETRY_DESCRIPTOR_H_
/// @brief   This file contains the definition of the GeometryDescriptor class
/// @note This file contains the definition of the GeometryDescriptor class, which is used to store and manage drawable objects, including points, lines, triangles, and other primitives.
/// @note The class is designed to be used with OpenGL, but it can be used with other rendering systems as well.
/// Provides a standard interface for rendering engines to use.

/// @dependencies 
/// @note   This class depends on the following libraries:
/// @note - STL (for data structures and algorithms)

/// @note      Can be used independently as a standalone class


#include <iostream>
#include <cstdint>
#include <string>
#include <array>
#include <vector>
#include <deque>
#include <unordered_map>
#include <memory>

#include "gl_typedefs.h"

#include "export.h"

/*
 * Functions
 */

/// @brief   An Inheritable Class for drawable objects ranging from simple points to complex 3D models
/// @note This class is used to store and manage drawable objects, including points, lines, triangles, and other primitives.
/// @note The class is designed to be used with OpenGL, but it can be used with other rendering systems as well.
/// Provides a standard interface for rendering engines to use.
/// Currently, the class supports only vertex positions, normals, and colors. Texture coordinates and other attributes can be added in the future.
namespace gridpro_gui {

class LIB_API GeometryDescriptor 
{
public:
    struct LIB_API PrimitiveSetInstance {
        /// @brief Enumeration for primitive types
        enum PrimitiveType {
            NONE           = GL_NONE_NULL,
            POINTS         = GL_POINTS,
            LINES          = GL_LINES,
            LINE_STRIP     = GL_LINE_STRIP,
            LINE_LOOP      = GL_LINE_LOOP,
            TRIANGLES      = GL_TRIANGLES,
            TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
            TRIANGLE_FAN   = GL_TRIANGLE_FAN,
            QUADS          = GL_QUADS,
            QUAD_STRIP     = GL_QUAD_STRIP,
            POLYGON        = GL_POLYGON
        };

        /// @brief Enumeration for color formats
        enum ColorFormat {
            RGB  = GL_RGB,
            RGBA = GL_RGBA
        };

        /// @brief Enumeration for color schemes        
        enum ColorScheme {
            PER_VERTEX = GL_COLOR_PER_VERTEX, 
            PER_PRIMITIVE = GL_COLOR_PER_PRIMITIVE,
            PER_PRIMITIVE_SET = GL_COLOR_PER_PRIMITIVE_SET
        };

        /// @brief  Enumeration for Shading Model
        enum ShadingModel {
            FLAT   = GL_FLAT,
            SMOOTH = GL_SMOOTH
        };

        enum WireframeMode {
            WIREFRAME_NONE = GL_NONE,
            WIREFRAME_ONLY = GL_WIREFRAME_ONLY,
            WIREFRAME_OVERLAY = GL_WIREFRAME_OVERLAY
        };       

        /// @brief  Enumeration for Material Properties
        enum MaterialProperty {
            AMBIENT   = GL_AMBIENT,
            DIFFUSE   = GL_DIFFUSE,
            SPECULAR  = GL_SPECULAR,
            SHININESS = GL_SHININESS,
            EMISSION  = GL_EMISSION,
            COLOR_MATERIAL = GL_COLOR_MATERIAL
        };

        enum Blendfunc {
            BLEND_NONE = GL_NONE,
            BLEND_SRC_ALPHA = GL_SRC_ALPHA,
            BLEND_ONE_MINUS_SRC_ALPHA = GL_ONE_MINUS_SRC_ALPHA
        }; 

        /// @brief  Enumeration for Pick Schemes
        enum PickScheme : GLenum {
            PICK_NONE = GL_PICK_NONE,
            PICK_BY_VERTEX = GL_PICK_BY_VERTEX,
            PICK_BY_PRIMITIVE = GL_PICK_BY_PRIMITIVE,
            PICK_GEOMETRY = GL_PICK_GEOMETRY
        };

        /// @brief  Enumeration for vertex array types
        enum VertexArrayType {
            POSITION_ARRAY = GL_VERTEX_ARRAY,
            NORMAL_ARRAY   = GL_NORMAL_ARRAY,
            COLOR_ARRAY    = GL_COLOR_ARRAY,
            INDEX_ARRAY    = GL_INDEX_ARRAY
        };

        /// @brief  Enumeration for dirty flags
        enum DirtyFlags {
            DIRTY_NONE           = 0,
            DIRTY_POSITIONS      = 1 << 0,
            DIRTY_NORMALS        = 1 << 1,
            DIRTY_COLORS         = 1 << 2,
            DIRTY_INDICES        = 1 << 3,
            DIRTY_SHADE_MODEL    = 1 << 4,
            DIRTY_COLOR_SCHEME   = 1 << 5,
            DIRTY_PICK_SCHEME    = 1 << 6,
            DIRTY_ALL            = DIRTY_POSITIONS | DIRTY_NORMALS | DIRTY_COLORS | DIRTY_INDICES | DIRTY_SHADE_MODEL | DIRTY_COLOR_SCHEME | DIRTY_PICK_SCHEME
            
        };

        struct unique_color_reservation {
            uint32_t EntityID;
            uint32_t start, end;
        }; 
   
        PrimitiveSetInstance(const std::string& _InstanceName,  GLenum _PrimitiveType) 
        :
          InstanceName(_InstanceName), primitiveType(static_cast<PrimitiveType>(_PrimitiveType)),
          
          colorFormat(RGB), colorScheme(PER_PRIMITIVE_SET), shadingModel(FLAT), wireframeMode(WIREFRAME_NONE), 
          
          materialProperty(COLOR_MATERIAL), blendfunc(BLEND_NONE), pickScheme(PICK_NONE), dirtyFlags(DIRTY_NONE), 
          
          is_hover_highlightable(false) , is_already_hover_highlighted(false), 
          
          is_select_highlightable(false), is_select_highlighted(false),
          
          is_using_custom_highlight_color(false),  is_node_manipulation_enabled(false),

          color(0, 0, 255, 255),   highlightcolor(255, 255, 255, 255), selection_highlight_color(0,0,255,255), 
          
          custom_highlight_color(0, 255, 0, 255), wireframecolor(0, 0, 200, 255),

          line_width(1.0f), point_size(10.0f),
          
          material_ambient({0.1f, 0.1f, 0.1f, 1.0f}), material_diffuse({0.1f, 0.1f, 0.1f, 1.0f}),

          material_specular({0.3f, 0.3f, 0.8f, 1.0f}), material_emission({0.0f, 0.0f, 0.0f, 1.0f}) ,  material_shininess(32.0f)
        {
            positions = std::make_shared<std::vector<float>>(0);
            normals   = std::make_shared<std::vector<float>>(0);
            colors    = std::make_shared<std::vector<uint8_t>>(0);
            indices   = std::make_shared<std::vector<uint32_t>>(0);
            pickScheme = PICK_NONE;
        }

        virtual ~PrimitiveSetInstance() {}

        std::shared_ptr<PrimitiveSetInstance> clone()
        {
            std::shared_ptr<PrimitiveSetInstance> clone = std::make_shared<PrimitiveSetInstance>(InstanceName, static_cast<GLenum>(primitiveType));

            PrimitiveSetInstance& clone_instance = *clone;

            clone_instance.colorFormat = colorFormat;
            clone_instance.colorScheme = colorScheme;
            clone_instance.shadingModel = shadingModel;
            clone_instance.wireframeMode = wireframeMode;
            clone_instance.materialProperty = materialProperty;
            clone_instance.blendfunc = blendfunc;
            clone_instance.pickScheme = pickScheme;
            clone_instance.dirtyFlags = dirtyFlags;
            clone_instance.is_hover_highlightable = is_hover_highlightable;
            clone_instance.is_already_hover_highlighted = is_already_hover_highlighted;
            clone_instance.is_select_highlightable = is_select_highlightable;
            clone_instance.is_select_highlighted = is_select_highlighted;
            clone_instance.is_using_custom_highlight_color = is_using_custom_highlight_color;
            clone_instance.is_node_manipulation_enabled = is_node_manipulation_enabled;
            clone_instance.color = color;
            clone_instance.highlightcolor = highlightcolor;
            clone_instance.selection_highlight_color = selection_highlight_color;
            clone_instance.custom_highlight_color = custom_highlight_color;
            clone_instance.wireframecolor = wireframecolor;
            clone_instance.line_width = line_width;
            clone_instance.point_size = point_size;
            clone_instance.material_ambient = material_ambient;
            clone_instance.material_diffuse = material_diffuse;
            clone_instance.material_specular = material_specular;
            clone_instance.material_emission = material_emission;
            clone_instance.material_shininess = material_shininess;
            clone_instance.positions = std::make_shared<std::vector<float>>(*positions);
            clone_instance.normals = std::make_shared<std::vector<float>>(*normals);
            clone_instance.colors = std::make_shared<std::vector<uint8_t>>(*colors);
            clone_instance.indices = std::make_shared<std::vector<uint32_t>>(*indices);
            return clone;
        }

        /*
        @warning 
        Do not use the following function to set primitive type. Once set in constructor its immutable
        @warning Do not use the following function to set the primitive type
        +--------------------------------------------------------------------------------------------------+
        Following function is used to set the primitive type
        /// @brief  Set Primitive type for the current PrimitiveSet
     
        /// @brief Set the primitive type
        void set_primitive_type(PrimitiveType type)   { primitiveType = type;  }
        void set_primitive_type(GLenum type)          { primitiveType = static_cast<PrimitiveType>(type); }
        */

       /// @brief Get the Instance Name
        std::string get_instance_name() const   { return InstanceName; }

        /// @brief  Get the primitive type
        PrimitiveType get_primitive_type() const { return primitiveType; }
        GLenum get_primitive_type_enum() const   { return static_cast<GLenum>(primitiveType); }

        /// @brief Set the color format
        /// @param format 
        void set_color_format(const ColorFormat& format)     { colorFormat = format; }
        void set_color_format(GLenum format)                 { colorFormat = static_cast<ColorFormat>(format); }
        
        /// @brief  Get the color format
        ColorFormat get_color_format() const    { return colorFormat; }
        GLenum get_color_format_enum() const    { return static_cast<GLenum>(colorFormat); }

        /// @brief Set the color scheme
        /// @param scheme
        void set_color_scheme(const ColorScheme& scheme)     { colorScheme = scheme; }
        void set_color_scheme(const uint32_t scheme)         { colorScheme = static_cast<ColorScheme>(scheme); }
        
        /// @brief Get the color scheme
        /// @return PrimitiveSetInstance::ColorScheme
        ColorScheme get_color_scheme() const    { return colorScheme; }
        GLenum get_color_scheme_enum() const    { return static_cast<GLenum>(colorScheme); }

        /// @brief Set the pick scheme
        /// @param scheme
        void set_pick_scheme(const PickScheme& scheme)       { pickScheme = scheme; }
        void set_pick_scheme(uint32_t scheme)                { pickScheme = static_cast<PickScheme>(scheme); }
        
        void swap_pick_scheme(uint32_t in_scheme)            
        {
            cache_pick_scheme = pickScheme; 
            pickScheme = static_cast<PickScheme>(in_scheme); 
        }

        void restore_pick_scheme()                          { pickScheme = cache_pick_scheme; }

        /// @brief Get the pick scheme
        PickScheme get_pick_scheme() const      { return pickScheme; }
        GLenum get_pick_scheme_enum() const     { return static_cast<GLenum>(pickScheme); }   

        /// @brief Get Pickable entites count for the current PrimitiveSet
        size_t get_pickable_entities_count() const
        {
            size_t count = 0;
            switch(pickScheme)
            {
                case PICK_NONE: count = 0; break;
                case PICK_BY_VERTEX: count = get_num_vertices(); break;
                case PICK_BY_PRIMITIVE: count = get_num_primitives(); break;
                case PICK_GEOMETRY: count = 1; break;
                default: count = 0; break;
            }
            return count;
        }

        /// @brief Set Min and Max Pickable entites for the current PrimitiveSet
        /// @param const min(starting index) generated by Scene_Manager
        /// @param max(ending index) evaluated and passed to Scene_Manager

        void set_pickable_entities_range(const size_t& min, size_t& max)
        {
            pick_color_reservation.start = min + 1;
            uint32_t reserve = get_pickable_entities_count();
            if(reserve == 1) reserve = 0;
            max = min + reserve;
            pick_color_reservation.end = max;
        }

        /// @brief Get Min and Max Pickable entites for the current PrimitiveSet
        /// @param const min(starting index) generated by Scene_Manager
        /// @param max(ending index) evaluated and passed to Scene_Manager

        const unique_color_reservation & get_pickable_entities_range()
        {
            return pick_color_reservation;
        }

        /// @brief Get Picked Primitive
        /// @param const index of the picked primitive
        /// @return std::vector<float> containing the picked primitive
        std::vector<float> get_primitive(const size_t& index)
        {
            std::vector<float> primitive;
            
            for(size_t i = 0; i < get_num_vertices_per_primitive(); i++)
            {
                std::array<float, 3> vertex = get_primitive_vertex(i + (index * get_num_vertices_per_primitive()));
                primitive.push_back(vertex[0]);
                primitive.push_back(vertex[1]);
                primitive.push_back(vertex[2]);
            }
            return primitive;
        }

        /// @brief Get Picked Primitive Vertex
        /// @param const index of the picked primitive
        /// @return std::vector<float> containing the picked primitive
        std::vector<float> get_picked_primitive(const size_t& index)
        {
            if(pickScheme == PICK_BY_VERTEX)
            {
                std::vector<float> primitive;
                float* vertex = get_vertex_ref(index);
                primitive.push_back(vertex[0]);
                primitive.push_back(vertex[1]);
                primitive.push_back(vertex[2]);
                return primitive;
            }
            else if(pickScheme == PICK_BY_PRIMITIVE)
            {
                return get_primitive(index);
            }
            else if(pickScheme != PICK_NONE)
            {
                return get_primitive(index);
            }

            else
            {
                return {};
            }
        }

        std::array<float, 3> get_primitive_vertex(const uint32_t& index)
        {
            if(indices->size() == 0)
                return {(*positions)[index * 3], (*positions)[index * 3 + 1], (*positions)[index * 3 + 2]};
            else
                return {(*positions)[((*indices)[index]) * 3], (*positions)[((*indices)[index]) * 3 + 1], (*positions)[((*indices)[index]) * 3 + 2]};
        }

        float* get_vertex_ref(const uint32_t& index)
        {
            if(index < get_num_vertices())
                return &(*positions)[index * 3];
            else
                return nullptr;
        }


        std::shared_ptr<GeometryDescriptor> tesselate_primitve(const uint32_t& index, const uint32_t& tessellation_level)
        {
            if(get_primitive_type_enum() != GL_TRIANGLES)
                return nullptr;

            std::shared_ptr<GeometryDescriptor> tesselated_primitive = std::make_shared<GeometryDescriptor>();
            
            std::vector<float> primitive = get_primitive(index);
            
            struct Point { float x, y, z; };

            auto calculateMidpoint = [](const Point &p1, const Point &p2) -> Point
            {
                return {(p1.x + p2.x) / 2, (p1.y + p2.y) / 2, (p1.z + p2.z) / 2};
            };
         
            auto triangle_tesslator = [&](std::array<Point, 3>& parent_triangle, uint32_t tessellation_level) -> std::vector<float>
            {
                std::vector<Point> tessellated_vertices;

                auto tessellate = [&](auto&& self, const std::array<Point, 3> &triangle, int level)
                {
                    if (level == 0)
                    {
                        // Add the vertices of the triangle to the output
                        tessellated_vertices.insert(tessellated_vertices.end(), triangle.begin(), triangle.end());
                        return;
                    }

                    // Calculate midpoints of the edges
                    Point mid1 = calculateMidpoint(triangle[0], triangle[1]);
                    Point mid2 = calculateMidpoint(triangle[1], triangle[2]);
                    Point mid3 = calculateMidpoint(triangle[2], triangle[0]);

                    // Create 4 new triangles
                    std::array<Point, 3> t1 = {triangle[0], mid1, mid3};
                    std::array<Point, 3> t2 = {mid1, triangle[1], mid2};
                    std::array<Point, 3> t3 = {mid3, mid2, triangle[2]};
                    std::array<Point, 3> t4 = {mid1, mid2, mid3};

                    // Recursively tessellate the new triangles
                    self(self, t1, level - 1);
                    self(self, t2, level - 1);
                    self(self, t3, level - 1);
                    self(self, t4, level - 1); 
                };

                // Start tessellation
                tessellate(tessellate, parent_triangle, tessellation_level);
                
                std::vector<float> tessellated_float_vertices(tessellated_vertices.size() * 3);

                for(size_t i = 0; i < tessellated_vertices.size(); i++)
                {
                    tessellated_float_vertices[i * 3] = tessellated_vertices[i].x;
                    tessellated_float_vertices[i * 3 + 1] = tessellated_vertices[i].y;
                    tessellated_float_vertices[i * 3 + 2] = tessellated_vertices[i].z;
                }
        
                return tessellated_float_vertices;
            };

            tesselated_primitive->set_current_primitive_set("Tesselated_Primitive", get_primitive_type_enum());
            
            if(get_primitive_type_enum() == GL_TRIANGLES)
            {
                  std::vector<float> all_tessellated_triangles = 
                  triangle_tesslator(reinterpret_cast<std::array<Point, 3> &>(primitive[0]), tessellation_level);
                  tesselated_primitive->move_pos_array(std::move(all_tessellated_triangles));
                  tesselated_primitive->set_pick_scheme(GL_PICK_BY_PRIMITIVE);
            }
            
            return tesselated_primitive;
        }
        
        /// @brief Set shader model
        /// @param model
        void set_shading_model(const ShadingModel& model)    { shadingModel = model; }
        void set_shading_model(const GLenum& model)          { shadingModel = static_cast<ShadingModel>(model); }

        /// @brief Get the shading model
        ShadingModel get_shading_model() const  { return shadingModel; }
        GLenum get_shading_model_enum() const   { return static_cast<GLenum>(shadingModel); }
        
        /// @brief Set Wireframe Mode
        /// @param mode
        void set_wireframe_mode(const WireframeMode& mode)    { wireframeMode = mode; }
        void set_wireframe_mode(const GLenum& mode)           { wireframeMode = static_cast<WireframeMode>(mode); }
        
        /// @brief Get Wireframe Mode
        WireframeMode get_wireframe_mode() const       { return wireframeMode; }
        GLenum get_wireframe_mode_enum() const         { return static_cast<GLenum>(wireframeMode); }

        /// @brief Set Blend Function
        /// @param func
        void set_blend_func(const Blendfunc& func)       { blendfunc = func; }
        void set_blend_func(GLenum func)                 { blendfunc = static_cast<Blendfunc>(func); }
  
        /// @brief Get Blend Function
        Blendfunc get_blend_func() const                 { return blendfunc; }
        GLenum get_blend_func_enum() const               { return static_cast<GLenum>(blendfunc); }
    
        /// @brief Set Line Width
        void set_line_width(const float& width) { line_width = width; }
        /// @brief Get Line Width
        float get_line_width() const            { return line_width; }

        /// @brief Set Point Size
        void set_point_size(const float& size)  { point_size = size; }
        /// @brief Get Point Size
        float get_point_size() const            { return point_size; }


        /// @brief Set the Material Properties
        void set_material_property(const GLenum& property, const float& r = 0.5, const float& g = 0.5, const float& b = 0.5, const float& a = 1.0f) 
        {
            switch(property)
            {
                case AMBIENT:  material_ambient = {r, g, b, a}; break;
                case DIFFUSE:  material_diffuse = {r, g, b, a}; break;
                case SPECULAR: material_specular = {r, g, b, a}; break;
                case SHININESS: material_shininess = r; break;
                case EMISSION: material_emission = {r, g, b, a}; break;
                default: break;
            }
        }

        /// @brief Get the Material Properties
        const float* get_material_property(const GLenum& property) const
        {
            switch(property)
            {
                case AMBIENT:  return material_ambient.data();
                case DIFFUSE:  return material_diffuse.data();
                case SPECULAR: return material_specular.data();
                case SHININESS: return &material_shininess;
                case EMISSION: return material_emission.data();
                default: return nullptr;
            }
        }


        /// @brief Get Weak Pointer to the Positions
        std::weak_ptr<std::vector<float>> get_position_weak_ptr()  const   
        { return positions; }
        
        /// @brief Get Weak Pointer to the Normals
        std::weak_ptr<std::vector<float>> get_normals_weak_ptr()    const  
        { return normals;  }
        
        /// @brief Get Weak Pointer to the Colors
        std::weak_ptr<std::vector<uint8_t>> get_colors_weak_ptr()   const  
        { return colors;   }

        /// @brief Get Weak Pointer to the Indices
        std::weak_ptr<std::vector<uint32_t>> get_indices_weak_ptr() const  
        { return indices;  }

        /// @brief Share Pointer to the Positions
        void share_position_shared_ptr(std::shared_ptr<std::vector<float>>& in_position) 
        { positions = in_position; }

        /// @brief Share Pointer to the Normals
        void share_normals_shared_ptr(std::shared_ptr<std::vector<float>>& in_normal) 
        { normals = in_normal; }

        /// @brief Share Pointer to the Colors
        void share_colors_shared_ptr(std::shared_ptr<std::vector<uint8_t>>& in_color) 
        { colors = in_color; }

        /// @brief SharePointer to the Indices
        void share_indices_shared_ptr(std::shared_ptr<std::vector<uint32_t>>& in_indices) 
        { indices = in_indices; }


        /// @brief Get the number of vertices
        size_t get_num_unique_positions() const { return positions->size() / 3; }
        size_t get_num_vertices() const         { return indices->size() ? indices->size() : positions->size() / 3; }
        size_t get_num_indices()  const         { return indices->size(); }
        size_t get_num_normals()  const         { return normals->size() / 3; }
        size_t get_num_colors()   const         { return colors->size() / (colorFormat == RGB ? 3 : 4); }
        size_t get_num_vertices_per_primitive() const 
        {
            GLuint vertices_per_primitive = 1 ;
            switch(primitiveType) {
                case POINTS:         vertices_per_primitive = 1; break;
                case LINES:          vertices_per_primitive = 2; break;
                case LINE_STRIP:     vertices_per_primitive = 2; break;
                case LINE_LOOP:      vertices_per_primitive = 2; break;
                case TRIANGLES:      vertices_per_primitive = 3; break;
                case TRIANGLE_STRIP: vertices_per_primitive = 3; break;
                case TRIANGLE_FAN:   vertices_per_primitive = 3; break;
                case QUADS:          vertices_per_primitive = 4; break;
                case QUAD_STRIP:     vertices_per_primitive = 4; break;
                case POLYGON:        vertices_per_primitive = 3; break;
                case NONE:           throw std::runtime_error("PrimitiveType is set to None\n"); break;  
                default:             vertices_per_primitive = 1; break;
            }
            return vertices_per_primitive;
        }

        size_t get_num_primitives() const       { return get_num_vertices() / get_num_vertices_per_primitive(); }

        /// @brief Reset the primitive set
        void release_ref_all() {

            positions.reset(); normals.reset();
            colors.reset();    indices.reset();
            
            positions = std::make_shared<std::vector<float>>(0);
            normals   = std::make_shared<std::vector<float>>(0);
            colors    = std::make_shared<std::vector<uint8_t>>(0);
            indices   = std::make_shared<std::vector<uint32_t>>(0);

            dirtyFlags = DIRTY_ALL;
        }

        /// @brief clear the primitive set
        void clear_all() {
            clear_positions(); clear_normals(); 
            clear_colors();    clear_indices();
        }        
        
        void clear_positions() 
        { positions->resize(0); dirtyFlags |= DIRTY_POSITIONS; }

        void clear_normals() 
        { normals->resize(0);   dirtyFlags |= DIRTY_NORMALS;   }

        void clear_colors() 
        { colors->resize(0);    dirtyFlags |= DIRTY_COLORS;    }

        void clear_indices() 
        { indices->resize(0);   dirtyFlags |= DIRTY_INDICES;   }

        void release_positions_ref() 
        { positions.reset(); positions = std::make_shared<std::vector<float>>(0);     dirtyFlags |= DIRTY_POSITIONS; }

        void release_normals_ref() 
        { normals.reset();   normals   = std::make_shared<std::vector<float>>(0);     dirtyFlags |= DIRTY_NORMALS;   }

        void release_colors_ref() 
        { colors.reset();    colors    = std::make_shared<std::vector<uint8_t>>(0);   dirtyFlags |= DIRTY_COLORS;    }

        void release_indices_ref() 
        { indices.reset();   indices   = std::make_shared<std::vector<uint32_t>>(0);  dirtyFlags |= DIRTY_INDICES;   }
        

        /// @brief Get Dirty Flags
        /// @param Dirtyflag 
        /// @return bool
        bool isDirty(DirtyFlags flag) const      { return (dirtyFlags & static_cast<int32_t>(flag)) != 0; }
        bool isDirty(const uint32_t& flag) const { return (dirtyFlags & flag) != 0; }
        bool isDirty() const                     { return  dirtyFlags != 0; }

        bool isHavingPositonUpdates() const     { return batch_vertex_updates.empty() == false; }

        void set_node_manipulator(const bool& flag)   { is_node_manipulation_enabled = flag; pickScheme = PICK_BY_VERTEX; }
        bool isNodeManipulationEnabled() const        { return is_node_manipulation_enabled; }

        /// @brief Set and Clear Dirty Flags
        /// @param flag
        void setDirty(DirtyFlags flag)                { dirtyFlags |= static_cast<int32_t>(flag); }
        void setDirty(const uint32_t& flag)           { dirtyFlags |= flag; }
        
        uint32_t getDirtyFlags() const                { return dirtyFlags; }

        /// @brief Clear Dirty Flags of a specific flag
        void clearDirty(DirtyFlags flag)              { dirtyFlags &= ~static_cast<int32_t>(flag); }
        void clearDirty(const uint32_t& flag)         { dirtyFlags &= ~flag; }
        
        /// @brief Clear all dirty flags
        void clearDirty()                             { dirtyFlags = 0; }

        /// @brief Validate the primitive set
        bool isDrawable() const                 { return positions->size() > 0 && primitiveType != PrimitiveType::NONE; }
        bool isHighlightable() const            { return is_hover_highlightable; }
        bool isHighlighted() const              { return is_already_hover_highlighted; }
        bool isSelectionHighlightable() const   { return is_select_highlightable; }
        bool isSelectionHighlighted() const     { return is_select_highlighted; }

        bool isUsingCustomHighlightColor() const { return is_using_custom_highlight_color; }


        /// @brief Set Highlightable
        void set_hover_highlightable(const bool& highlightable) { is_hover_highlightable = highlightable; }
        
        /// @brief Set hover Highlighted
        void set_hover_highlights(const bool& highlight_flag)     
        {
            if(isSelectionHighlighted()) 
            return;

            if(is_hover_highlightable == true) 
            {
              if(highlight_flag == true && is_already_hover_highlighted == false) 
              {
                is_already_hover_highlighted = true; 
                color.swap(highlightcolor);
              }
              else if(highlight_flag == false && is_already_hover_highlighted == true)
              {
                is_already_hover_highlighted = false;
                color.swap(highlightcolor);
              }
            }
        }

        /// @brief Set Selected Highlighted
        void set_selection_highlightable(const bool& selection_highlightable) { is_select_highlightable = selection_highlightable; }

        /// @brief Set Selected Highlighted
        void set_selection_highlights(const bool& selection_highlight_flag)     
        {
            if(isHighlighted()) 
            {
                color.swap(highlightcolor);
                is_already_hover_highlighted = false;
            }
            if(is_select_highlightable == true)
            {
             if(selection_highlight_flag == true && is_select_highlighted == false) 
             {
                is_select_highlighted = selection_highlight_flag; 
                color.swap(selection_highlight_color);
             }
             else if(selection_highlight_flag == false && is_select_highlighted == true)
             {
                is_select_highlighted = selection_highlight_flag;
                color.swap(selection_highlight_color);
             }
            }
        }

        /// @brief Set the color
        void set_selection_color(const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a) 
        { 
           selection_highlight_color = {r, g, b, a}; 
        } 
        
        /// @brief Set the custom highlight color
        void set_custom_highlight_color(const uint8_t& r, const uint8_t& g, const uint8_t& b, const uint8_t& a) 
        { 
           custom_highlight_color = {r, g, b, a}; 
        }
        
        std::array<float, 6> get_bounding_box() const 
        {
            if(m_bounding_box) return (*m_bounding_box);

            struct point { float x, y, z;};
            point min, max;

            for(int i = 0;  i < positions->size() ; i += 3)
            {
               const auto& pos = *positions;
               if(min.x > pos[0]) min.x = pos[0]; 
               if(min.x > pos[1]) min.x = pos[1]; 
               if(min.x > pos[2]) min.x = pos[2]; 
               
               if(max.x < pos[0]) max.x = pos[1]; 
               if(max.y < pos[1]) max.x = pos[2]; 
               if(max.y < pos[2]) max.x = pos[3]; 
            }
            
            return {min.x , min.y, min.z, max.x, max.y , max.z}; 
        } 

        void set_bounding_box(std::array<float, 6> in_bounding_box) 
        {
            m_bounding_box = std::make_shared<std::array<float, 6>>(in_bounding_box);
        }

        /// @brief Validate the primitive set (will throw an exception if the primitive set is not valid)
        /// @note This function is used to validate the primitive set
        /// @note It will throw an exception if the primitive set is not valid
        /// @return bool
        /// @throws std::runtime_error
        /// @warning Do not Modify ! This Function will provide necessary information about the error in the exception message
        bool isValid()                     
        {   
            std::string err;
            bool valid = true;
            valid &= isDrawable();
            if(positions->size() % 3 != 0) valid = false;
            if(get_num_indices() > 0) valid &= get_num_indices() % get_num_vertices_per_primitive() == 0;
     
            if(get_num_normals() > 0) 
            {
                //flatten_normal_array();
                valid &= (get_num_normals() == get_num_vertices() || get_num_normals() == get_num_primitives());
            }

            if(get_num_colors()  > 0) valid &= get_num_colors()  == get_num_vertices();

            if (!valid) {
                std::string errorMessage = std::string("GeometryDescriptor with ID: [") + InstanceName + std::string("] failed validation due to :");
                if (!isDrawable()) {
                    errorMessage += "\n- No drawable primitives ";
                    if(positions->size() == 0)
                       errorMessage += "\n-  No positions !!! (positions->size() == 0) !!!";
                    if(primitiveType == PrimitiveType::NONE)
                       errorMessage += "\n-  No primitive type set !!! (primitiveType == NONE) !!!";
                }
                if (positions->size() % 3 != 0) {
                    errorMessage += "\n- Invalid number of positions (not a multiple of 3)";
                    errorMessage += "\n- positions->size() = " + std::to_string(positions->size());
                }
                if (get_num_indices() > 0 && get_num_indices() % get_num_vertices_per_primitive() != 0) {
                    errorMessage += "\n- Invalid number of indices (not a multiple of vertices per primitive)";
                    errorMessage += "\n- get_num_indices() = " + std::to_string(get_num_indices());
                }
                if (get_num_normals() > 0 && get_num_normals() != get_num_vertices()) {
                    errorMessage += "\n- Invalid number of normals (not equal to number of vertices)";
                    errorMessage += "\n- get_num_normals() = " + std::to_string(get_num_normals());
                    errorMessage += "\n- get_num_vertices() = " + std::to_string(get_num_vertices());
                }
                if (get_num_colors() > 0 && get_num_colors() != get_num_vertices()) {
                    errorMessage += "\n- Invalid number of colors (not equal to number of vertices)";
                    errorMessage += "\n- get_num_colors() = " + std::to_string(get_num_colors());
                    errorMessage += "\n- get_num_vertices() = " + std::to_string(get_num_vertices());
                }

                throw std::runtime_error(errorMessage);
            }

            return valid;
            } 
             
           /// @brief Get Positions Vector const ref
           /// @warning Do not const cast !!!
           const std::vector<float>& positions_vector() {  return *positions;  } 

           /// @brief Get Normals Vector
           /// @warning Do not const cast !!!
           const std::vector<float>& normals_vector()   { return *normals; }   

           /// @brief Get Colors Vector
           /// @warning Do not const cast !!!
           const std::vector<uint8_t>& colors_vector()  { return *colors;} 

           /// @brief Get Indices Vector
           /// @warning Do not const cast !!!
           const std::vector<uint32_t>& indices_vector() { return *indices;}   
           
           void flatten_postion_array()
           {
            if(indices_vector().size() == 0) return;
            
            std::vector<float> temp_positions(get_num_vertices() * 3);
            
            auto& indices = indices_vector();
            auto& positions = positions_vector();
            const uint32_t num_vertices = get_num_vertices();
            for(size_t i = 0; i < num_vertices; i++)
            {
                temp_positions[i * 3 + 0] = positions[indices[i] * 3 + 0];
                temp_positions[i * 3 + 1] = positions[indices[i] * 3 + 1];
                temp_positions[i * 3 + 2] = positions[indices[i] * 3 + 2];
            }
             
            *(this->positions) = (std::move(temp_positions));
              release_indices_ref();
           }
           
           void flatten_normal_array()
           {
                if(indices_vector().size() == 0) return;
                
                if(normals->size() == 0) return;
                
                if(normals->size() != positions->size())
                {
                    throw std::runtime_error("Normals size is not equal to positions size");
                }

                if(normals->size() != indices_vector().size() * 3)
                {

                    std::vector<float> temp_normals(indices_vector().size() * 3);
                    
                    auto& indices = indices_vector();
                    auto& normals = normals_vector();
                    const uint32_t num_vertices = get_num_vertices();
                    for(size_t i = 0; i < num_vertices; i++)
                    {
                        temp_normals[i * 3 + 0] = normals[indices[i] * 3 + 0];
                        temp_normals[i * 3 + 1] = normals[indices[i] * 3 + 1];
                        temp_normals[i * 3 + 2] = normals[indices[i] * 3 + 2];
                    }
                    
                    *(this->normals) = (std::move(temp_normals));
                }
           }
            
           std::vector<float> get_flattened_position_array()
           {
                if(indices_vector().size() == 0) return positions_vector();
                std::vector<float> temp_positions(get_num_vertices() * 3);
                
                auto& indices = indices_vector();
                auto& positions = positions_vector();
                const uint32_t num_vertices = get_num_vertices();
                for(size_t i = 0; i < num_vertices; i++)
                {
                    temp_positions[i * 3 + 0] = positions[indices[i] * 3 + 0];
                    temp_positions[i * 3 + 1] = positions[indices[i] * 3 + 1];
                    temp_positions[i * 3 + 2] = positions[indices[i] * 3 + 2];
                }
                return temp_positions;
           }


           void update_vertex(const std::array<float, 3>& position, const uint32_t& index)
           {
                float *vertex  = get_vertex_ref(index);
                if(vertex != nullptr)
                {
                    vertex[0] = position[0];
                    vertex[1] = position[1];
                    vertex[2] = position[2];
                }

                batch_vertex_update update;
                update.m_position = position;
                update.index = index;
                batch_vertex_updates.push_back(update);
           }

            private : 
            friend class GeometryDescriptor;
             /// @brief Name of the primitive set instance
            const std::string   InstanceName;
            
            /// @brief Primitive type (e.g., GL_TRIANGLES, GL_LINES, etc.)
            const PrimitiveType primitiveType; 
            
            /// @brief RGB or RGBA values for each vertex
            ColorFormat colorFormat; 

            /// @brief Color scheme for the primitive set
            ColorScheme colorScheme; 

            /// @brief Shading model for the primitive set
            ShadingModel shadingModel; 

            /// @brief Wireframe mode for the primitive set
            WireframeMode wireframeMode;

            /// @brief Material properties for the primitive set
            MaterialProperty materialProperty; 

            /// @brief Blend function for the primitive set
            Blendfunc blendfunc;

            /// @brief Pick scheme for the primitive set
            PickScheme pickScheme;  
            
            // @brief Pick Scheme cache for the primitive set
            PickScheme cache_pick_scheme;

            /// @brief Pick color reservation for the primitive set
            struct unique_color_reservation  pick_color_reservation; 
        
            /// @brief Positions for this primitive set
            std::shared_ptr<std::vector<float>>    positions;

            /// @brief Normals for this primitive set
            std::shared_ptr<std::vector<float>>    normals;

             /// @brief RGB or RGBA values for each vertex
            std::shared_ptr<std::vector<uint8_t>>  colors;

            /// @brief Indices for this primitive set
            std::shared_ptr<std::vector<uint32_t>> indices;   

            /// @brief Bounding Box
            std::shared_ptr<std::array<float, 6>> m_bounding_box;
        
            /// @brief Flags to indicate which data has changed
            uint32_t dirtyFlags; 
            
            public :
            /// @brief Color if(if Mono Color Scheme)
            struct Color { 
               uint8_t r, g, b, a;
               Color(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a) : r(_r), g(_g), b(_b), a(_a) {}
               Color() : r(255), g(255), b(255), a(255) {}
               Color(const Color& other) : r(other.r), g(other.g), b(other.b), a(other.a) {}
              ~Color() {}
               std::array<float, 4> get_color()             { return {r/255.0f, g/255.0f, b/255.0f, a/255.0f}; }
               std::array<float, 4> get_color_float_array() { return get_color(); }

               void swap(Color& other)
               {
                  std::swap(r, other.r);
                  std::swap(g, other.g);
                  std::swap(b, other.b);
                  std::swap(a, other.a);
               }

                Color& operator=(const Color& other)
                {
                     if(this != &other)
                     {
                        Color temp(other);
                        swap(temp);
                     }
                     return *this;
                }
            };

            /// @Material Properties
            std::array<float, 4> material_ambient;
            std::array<float, 4> material_diffuse;
            std::array<float, 4> material_specular;
            std::array<float, 4> material_emission;
            float material_shininess;

            /// @brief Batch Vertex Update
            struct batch_vertex_update
            {
                std::array<float, 3> m_position;
                uint32_t index;
            };

            std::deque<batch_vertex_update> batch_vertex_updates; 
      
            bool  is_hover_highlightable; 
            bool  is_already_hover_highlighted;

            bool  is_select_highlightable;
            bool  is_select_highlighted;
            
            bool  is_using_custom_highlight_color;

            bool  is_node_manipulation_enabled;

            Color color;
            Color highlightcolor; // hover highlight color
            Color selection_highlight_color;  // selected color
            Color custom_highlight_color;
            Color wireframecolor;
            
            float line_width;
            float point_size;
            };  // Struct PrimitiveSetInstance

    //+---------------------------------------------------------------------------------------------------------+
    #define primitive_set_iterator std::unordered_map<std::string, std::shared_ptr<PrimitiveSetInstance>>::iterator
    //+---------------------------------------------------------------------------------------------------------+
    /// @brief Member Variables of Geometry Descriptor
    std::unordered_map<std::string, std::shared_ptr<PrimitiveSetInstance>> primitives;
    std::string currentPrimitiveSetInstanceName;
    std::shared_ptr<PrimitiveSetInstance> currentPrimitiveSet;
    uint32_t id;
    //+---------------------------------------------------------------------------------------------------------+
    uint32_t get_id() const { return id; }
    void     set_id(uint32_t _id)     { id = _id;  }
    
    void set_color_id_reserve_start(uint32_t start)  {  currentPrimitiveSet->pick_color_reservation.start = start; }
    uint32_t  get_color_id_reserve_start()           {  return currentPrimitiveSet->pick_color_reservation.start;  }
    uint32_t get_color_id_reserve_end()        
    {   
        uint32_t reserve = currentPrimitiveSet->get_pickable_entities_count();
        if(reserve == 1) reserve = 0;
        currentPrimitiveSet->pick_color_reservation.end  = currentPrimitiveSet->pick_color_reservation.start + reserve;   
        return currentPrimitiveSet->pick_color_reservation.end; 
    }
    

    /// @brief Constructor
    GeometryDescriptor();
    
    /// @brief Destructor
    virtual ~GeometryDescriptor();
    
    /// @brief Assignment operator
    GeometryDescriptor& operator=(const GeometryDescriptor& src) 
    {        
        primitives = src.primitives;
        currentPrimitiveSetInstanceName = src.currentPrimitiveSetInstanceName;
        currentPrimitiveSet = src.currentPrimitiveSet;
        id = src.id;
        
        return *this;
    }
    
    /// @brief Copy Constructor
    GeometryDescriptor(const GeometryDescriptor& src) 
    {
        primitives = src.primitives;
        currentPrimitiveSetInstanceName = src.currentPrimitiveSetInstanceName;
        currentPrimitiveSet = src.currentPrimitiveSet;
        id = src.id;
    }

    __INLINE__ std::shared_ptr<GeometryDescriptor> clone() 
    {
        std::shared_ptr<GeometryDescriptor> clone_instance = std::make_shared<GeometryDescriptor>();
        GeometryDescriptor& clone = *clone_instance;
        for(auto& primitive : primitives)
        {
            clone.primitives[primitive.first] = primitive.second->clone();
        }
        clone.currentPrimitiveSetInstanceName = currentPrimitiveSetInstanceName;
        clone.currentPrimitiveSet = clone.primitives[clone.currentPrimitiveSetInstanceName];
        clone.id = id;

        return clone_instance;
    } 

    __INLINE__ void swap_pick_scheme(uint32_t pick_scheme)
    {
        for(auto& primitive : primitives)
        {
            primitive.second->swap_pick_scheme(pick_scheme);
        }
    }

    __INLINE__ void restore_pick_scheme()
    {
        for(auto& primitive : primitives)
        {
            primitive.second->restore_pick_scheme();
        }
    }
    
 /* 
    @brief 
    Functions for managing primitive sets
    @note
    These functions are used to manage primitive sets in the drawable object 
 */

    /// @brief Set the current primitive set
    __INLINE__ void set_new_primitive_set(const std::string& name, GLenum Primitivetype);

    /// @brief Set the current primitive set
    __INLINE__ void set_current_primitive_set(const std::string& name, GLenum Primitivetype);

    /// @brief get current primitive set name
    /// @return std::string
    __INLINE__ const std::string get_current_primitive_set_name() const { return currentPrimitiveSetInstanceName; }

    /// @brief get a primitive set by name
    /// @param name 
    /// @return std::weak_ptr<PrimitiveSetInstance>. call lock() to get a shared_ptr
    __INLINE__ std::weak_ptr<PrimitiveSetInstance> get_primitive_set(const std::string& name);
    
    /// @brief get the current primitive set
    /// @param name
    /// @return std::weak_ptr<PrimitiveSetInstance>. call lock() to get a shared_ptr
    __INLINE__ std::weak_ptr<PrimitiveSetInstance> get_current_primitive_set();

    /// @brief -> operator to get the current primitive set
    /// @return std::shared_ptr<PrimitiveSetInstance>
    /// @note This is useful when you want to access the current primitive set directly
    __INLINE__ std::shared_ptr<PrimitiveSetInstance>& operator->() { return currentPrimitiveSet; }

    /// @brief remove a primitive set by name
    /// @param name
    __INLINE__ void remove_primitive_set(const std::string& name);

    /// @brief remove all primitive sets
    __INLINE__ void remove_all_primitive_sets();

    /// @brief copy a primitive set
    __INLINE__ void copy_vertex_attributes(const std::string& src, const std::string& dst);
    
    /// @brief move a primitive set
    __INLINE__ void move_vertex_attributes(const std::string& src, const std::string& dst);

    /// @brief share a primitive set
    __INLINE__ void share_vertex_attributes(const std::string& src, const std::string& dst);

    /// @brief copy all primitive sets
    __INLINE__ void copy_all_primitive_sets(const GeometryDescriptor& src);
 
    /// @brief move all primitive sets
    __INLINE__ void move_all_primitive_sets(GeometryDescriptor&& src);

    /// @brief clear all primitive sets
    __INLINE__ void clear_all_primitive_sets();

    /// @brief  Get number of primitive sets
    /// @return size_t
    __INLINE__  size_t get_num_primitive_sets() const { return primitives.size(); }

    /// @brief  Get Begin iterator of the Primitive Set
    /// @return std::unordered_map<std::string, std::shared_ptr<PrimitiveSetInstance>>::iterator 
    __INLINE__ primitive_set_iterator begin() {return primitives.begin();}

    /// @brief  Get End iterator of the Primitive Set
    /// @return std::unordered_map<std::string, std::shared_ptr<PrimitiveSetInstance>>::iterator 
    __INLINE__ primitive_set_iterator end() {return primitives.end();}

/*
  @brief 
  Functions for adding vertex attributes data
  @note
  These functions are used to add vertex attributes data to the current primitive set
*/

    /// @brief Push a position vector (x, y, z) to the current primitive set
    __INLINE__ void push_pos3f(const float& x, const float& y, const float& z);

    /// @brief Push a normal vector (n1, n2, n3) to the current primitive set
    __INLINE__ void push_normal3f(const float& n1, const float& n2, const float& n3);

    /// @brief Push a color vector (r, g, b) to the current primitive set
    __INLINE__ void push_color3ub(const uint8_t& r, const uint8_t& g, const uint8_t& b);

    /// @brief Push a color vector (r, g, b, a) to the current primitive set
    __INLINE__ void push_color4ub(const uint8_t& r, const uint8_t& g, const uint8_t& b , const uint8_t a);

    /// @brief Push indices to the current primitive set
    __INLINE__ void push_index(const uint32_t& index);    

    /// @brief Push a position array to the current primitive set (inserts the array at the end of the current array)
    __INLINE__ void push_pos_array(const std::vector<float>& position_array);

    /// @brief Push a normal array to the current primitive set (inserts the array at the end of the current array)
    __INLINE__ void push_normal_array(const std::vector<float>& normal_array);

    /// @brief Push a color array to the current primitive set (inserts the array at the end of the current array)
    __INLINE__ void push_color_array(const std::vector<uint8_t>& color_array);

    /// @brief Push a Index array to the current primitive set (inserts the array at the end of the current array)
    __INLINE__ void push_index_array(const std::vector<uint32_t>& index_array);

    /// @brief Copy a position array to the current primitive set (replaces the current array)
    __INLINE__ void copy_pos_array(const std::vector<float>& position_array);

    /// @brief Copy a normal array to the current primitive set (replaces the current array)
    __INLINE__ void copy_normal_array(const std::vector<float>& normal_array);

    /// @brief Copy a color array to the current primitive set (replaces the current array)
    __INLINE__ void copy_color_array(const std::vector<uint8_t>& color_array);     

    /// @brief Copy a Index array to the current primitive set (replaces the current array)
    __INLINE__ void copy_index_array(const std::vector<uint32_t>& index_array);

    /// @brief Move a position array to the current primitive set (replaces the current array)
    __INLINE__ void move_pos_array(std::vector<float>&& position_array);

    /// @brief Move a normal array to the current primitive set (replaces the current array)
    __INLINE__ void move_normal_array(std::vector<float>&& normal_array);
     
    /// @brief Move a color array to the current primitive set (replaces the current array)
    __INLINE__ void move_color_array(std::vector<uint8_t>&& color_array);
    
    /// @brief Move a Index array to the current primitive set (replaces the current array)
    __INLINE__ void move_index_array(std::vector<uint32_t>&& index_array);

    /// @brief    copy by values vertex attrib array
    /// @note  Copy the vertex attrib array by values to another primitive set
    /// @note  This is useful when you want to copy the same vertex attrib array between multiple primitive sets
    /// @param src , 
    /// @param dst
    __INLINE__ void copy_attrib_array(const std::string& src, const std::string& dst, PrimitiveSetInstance::VertexArrayType type);

    /// @brief    copy by ref vertex attrib array
    /// @note  Copy the vertex attrib array as a reference to another primitive set
    /// @note  This is useful when you want to share the same vertex attrib array between multiple primitive sets
    /// @param src 
    /// @param dst 
    __INLINE__ void share_attrib_array(const std::string& src, const std::string& dst, PrimitiveSetInstance::VertexArrayType type);

    /// @brief    copy by values vertex attrib array
    /// @note  Copy the vertex attrib array by values to all other primitive set
    /// @note  This is useful when you want to copy the same vertex attrib array between multiple primitive sets
    /// @param src , 
    /// @param dst
    __INLINE__ void copy_attrib_array_with_all(const std::string& src, PrimitiveSetInstance::VertexArrayType type);

    /// @brief    copy by ref vertex attrib array
    /// @note  Copy the vertex attrib array as a reference to all other primitive set
    /// @note  This is useful when you want to share the same vertex attrib array between multiple primitive sets
    /// @param src
    /// @param dst
    __INLINE__ void share_attrib_array_with_all(const std::string &src, PrimitiveSetInstance::VertexArrayType type);

    /// @brief Share Positions Array by accepting a shared pointer to the positions array
    /// @param std::shared_ptr<std::vector<float>> in_position
    /// @note  This is useful when you want to share the same positions array between multiple descriptors
    __INLINE__ void share_positions_shared_ptr(std::shared_ptr<std::vector<float>> &in_position);

    /// @brief Share Normals Array by accepting a shared pointer to the normals array
    /// @param std::shared_ptr<std::vector<float>> in_normal
    /// @note  This is useful when you want to share the same normals array between multiple descriptors
    __INLINE__ void share_normals_shared_ptr(std::shared_ptr<std::vector<float>> &in_normal);

    /// @brief Share Colors Array by accepting a shared pointer to the colors array
    /// @param std::shared_ptr<std::vector<uint8_t>> in_color
    /// @note  This is useful when you want to share the same colors array between multiple descriptors
    __INLINE__ void share_colors_shared_ptr(std::shared_ptr<std::vector<uint8_t>> &in_color);

    /// @brief Share Indices Array by accepting a shared pointer to the indices array
    /// @param std::shared_ptr<std::vector<uint32_t>> in_indices
    /// @note  This is useful when you want to share the same indices array between multiple descriptors
    __INLINE__ void share_indices_shared_ptr(std::shared_ptr<std::vector<uint32_t>> &in_indices);

    /// @brief    Set the Bounding Box of the current primitive set
    /// @param std::array<float, 6> bounding_box
    __INLINE__ void set_bounding_box(const std::array<float, 6>& bounding_box);

    /// @brief    Set the fill color of the current primitive set
    /// @param uint8_t r, uint8_t g, uint8_t b, uint8_t a
    __INLINE__ void set_fill_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a);

    /// @brief    Set the Wireframe color of the current primitive set
    /// @param uint8_t r, uint8_t g, uint8_t b, uint8_t a
    __INLINE__ void set_wireframe_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a);

    /// @brief    Set the Highlight color of the current primitive set
    /// @param uint8_t r, uint8_t g, uint8_t b, uint8_t a
    __INLINE__ void set_highlight_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a); 
 
    /// @brief    Set the Selection Highlight color of the current primitive set
    /// @param uint8_t r, uint8_t g, uint8_t b, uint8_t a
    __INLINE__ void set_selection_highlight_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a);

    /// @brief    Set the Custom Highlight color of the current primitive set
    /// @param uint8_t r, uint8_t g, uint8_t b, uint8_t a
    __INLINE__ void set_custom_highlight_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a);

    /// @brief    Set the pick scheme of the current primitive set
    /// @param GLenum scheme
    __INLINE__ void set_pick_scheme(const GLenum scheme);

    /// @brief    Set the shading model of the current primitive set
    /// @param GLenum Shademodel
    __INLINE__ void set_shading_model(const GLenum model);

    /// @brief    Set the wireframe mode of the current primitive set
    /// @param GLenum Wireframemode
    __INLINE__ void set_wireframe_mode(const GLenum mode);

    /// @brief    Set the color scheme of the current primitive set
    /// @param GLenum ColorScheme
    __INLINE__ void set_color_scheme(const GLenum scheme);

    /// @brief    Set the color format of the current primitive set
    /// @param GLenum ColorFormat
    __INLINE__ void set_color_format(const GLenum format);

    /// @brief    Set the material property of the current primitive set
    /// @param GLenum MaterialProperty
    __INLINE__ void set_material_property(const GLenum property);

    /// @brief    Set the blend function of the current primitive set
    /// @param GLenum Blendfunc
    __INLINE__ void set_blend_func(const GLenum func);

    /// @brief Set the Highlight mode of the current primitive set
    /// @param bool
    __INLINE__ void set_hover_highlights(const bool& highlight_flag);

    /// @brief Set the Highlightable mode of the current primitive set
    /// @param bool
    __INLINE__ void set_hover_highlightable_mode(const bool& isHighlightable);

    /// @brief Set the Selection Highlight mode of the current primitive set
    /// @param bool
    __INLINE__ void set_selection_highlights(const bool& selection_highlight_flag);

    /// @brief Set the Selection Highlightable mode of the current primitive set
    /// @param bool
    __INLINE__ void set_custom_highlights(const bool& flag);

    /// @brief Set the Selection Highlightable mode of the current primitive set
    /// @param bool
    __INLINE__ void set_selection_highlightable_mode(const bool& isSelectHighlightable);

    /// @brief Set the Line Width of the current primitive set
    /// @param float
    __INLINE__ void set_line_width(const float& width);

    /// @brief Set the Point Size of the current primitive set
    /// @param float
    __INLINE__ void set_point_size(const float& size);

    /// @brief update the vertex position
    /// @param std::array<float, 3> position
    /// @param uint32_t index
    __INLINE__ void update_vertex(const std::array<float, 3>& position, const uint32_t& index);

    /// @brief translate the vertex position
    /// @param std::array<float, 3> translation_vector
    /// @param uint32_t index
    __INLINE__ void translate_vertex(const std::array<float, 3>& translation_vector, const uint32_t& index);

    /// @brief check if the Node Manipulation is enabled
    /// @return bool
    __INLINE__ bool isNodeManipulationEnabled() const;

    /// @brief Set the Node Manipulation
    /// @param bool
    __INLINE__ void set_node_manipulator(const bool& flag);

    /// @brief   Other methods for dirty flags, etc. can be added here
    /// @note clears all dirty flags for all primitive sets
    /// @warning Use -> operator to instead to clear the current primitive set dirty flags
    __INLINE__ void clearDirtyFlags();

    /// @brief  Clear dirty flags for a specific primitive set
    /// @param name 
    __INLINE__ void clearDirtyFlags(const std::string& name);

    /// @brief  Clear dirty flags for a specific primitive set and a specific attribute flag
    /// @param name 
    /// @param flag 
    __INLINE__ void clearDirtyFlags(const std::string& name, PrimitiveSetInstance::DirtyFlags flag) ;

    /// @brief  check if the current PrimitiveSet is drawable.
    /// @return bool
    __INLINE__ bool isDrawable() const;

    /// @brief  check if the geometry has any drawable primitives
    /// @return bool
    __INLINE__ bool hasAnyDrawables() const;

    /// @brief  check if the current PrimitiveSet is valid.
    /// @return bool
    __INLINE__ bool isValid() const;

    /// @brief check if the current Primtitveset is highlighted
    /// @return bool
    __INLINE__ bool isHighlighted() const;

    /// @brief Get the Bounding Box of the current primitive set
    /// @return std::array<float, 6>
    __INLINE__ std::array<float, 6> get_bounding_box() const;

};
}

#endif // _HLM_DRAWABLE_H_
