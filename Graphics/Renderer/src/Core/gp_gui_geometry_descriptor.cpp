#include <iostream>
#include "gp_gui_geometry_descriptor.h"
#include "gp_gui_debug.h"

namespace gridpro_gui {

    /// @brief Constructor
    GeometryDescriptor::GeometryDescriptor() : currentPrimitiveSetInstanceName("_DEFAULT_") 
    { 
        currentPrimitiveSet = std::make_shared<PrimitiveSetInstance>(currentPrimitiveSetInstanceName, GL_POINTS);
        primitives[currentPrimitiveSetInstanceName] = currentPrimitiveSet;
    }
    
    /// @brief Destructor
    GeometryDescriptor::~GeometryDescriptor() 
    {

    }

  /// @brief Set a new primitive set
  /// @param name 
  /// @param Primitivetype
  /// @details This function is used to set a new primitive set with a given name and primitive type
  /// @details If a primitive set with the same name already exists, it will be overwritten

    __INLINE__ void GeometryDescriptor::set_new_primitive_set(const std::string& name , GLenum Primitivetype) {
        currentPrimitiveSetInstanceName = name;
        if(primitives.find(name) != primitives.end() && primitives[name] != nullptr)
        {
            primitives[name].reset();
            GP_TRACE("Warning ! You are ovewriting an existing Primitive set with ID : ", name , "\n"); 
        }  
        primitives[name] = std::make_shared<PrimitiveSetInstance>(name, Primitivetype);

        currentPrimitiveSet = primitives[name];    
    }

  /// @brief Set the current primitive set
  /// @param name
  /// @param Primitivetype
  /// @details This function is used to set the current primitive set with a given name and primitive type from map
  /// @details If a primitive set with the same name already exists, it will be overwritten
    __INLINE__ void GeometryDescriptor::set_current_primitive_set(const std::string& name , GLenum Primitivetype = GL_NONE_NULL) {
        currentPrimitiveSetInstanceName = name;
        if(primitives.find(name) == primitives.end())
        {
           if(Primitivetype == GL_NONE_NULL) 
           {
              std::string err =  std::string("Warning ! You are creating a new Primitive set with ID : ") + name + std::string(" with no Primitive type. Set a Valid PrimitiveType\n");
              throw std::runtime_error(err);
           }
 
           primitives[name] = std::make_shared<PrimitiveSetInstance>(name, Primitivetype);
           GP_TRACE("Warning ! You are creating a new Primitive set with ID : ", name , ". Use set_new_primitive_set() instead if you create a new PrimitiveSet \n");
        }
        else
        {
            if(primitives[name]->get_primitive_type_enum() != Primitivetype)
               GP_TRACE("Warning ! You are trying to ovewrite an existing Primitive set with const Primitive type ID : ",  name ,
                          " !!!. Use set_new_primitive_set() instead if you create a new PrimitiveSet\n"); 
        }
        currentPrimitiveSet = primitives[name];    
    }

    /// @brief Push a position vector (x, y, z) to the current primitive set
    __INLINE__ void GeometryDescriptor::push_pos3f(const float& x, const float& y, const float& z) {
        
        auto& primitiveSet = currentPrimitiveSet;
        primitiveSet->positions->push_back(x);
        primitiveSet->positions->push_back(y);
        primitiveSet->positions->push_back(z);

        /// @brief   Set the dirty flag for positions
        /// @details This is used to indicate that the positions have been modified
        /// @warning You can remove this line for performance reasons only if you are sure that you manually set the dirty flag
        #ifdef _ENABLE_AUTOMATIC_DIRTY_FLAG_MANAGEMENT_
        primitiveSet->setDirty(PrimitiveSetInstance::DIRTY_POSITIONS);
        #endif
    }

    /// @brief Push a normal vector (n1, n2, n3) to the current primitive set
    __INLINE__ void GeometryDescriptor::push_normal3f(const float& n1, const float& n2, const float& n3) {
        auto& primitiveSet = currentPrimitiveSet;
        primitiveSet->normals->push_back(n1);
        primitiveSet->normals->push_back(n2);
        primitiveSet->normals->push_back(n3);

        /// @brief   Set the dirty flag for normals
        /// @details This is used to indicate that the normals have been modified
        /// @warning You can remove this line for performance reasons only if you are sure that you manually set the dirty flag
        #ifdef _ENABLE_AUTOMATIC_DIRTY_FLAG_MANAGEMENT_
        primitiveSet->setDirty(PrimitiveSetInstance::DIRTY_NORMALS);
        #endif
    }

    /// @brief Push a color vector (r, g, b) ubytes to the current primitive set
    __INLINE__ void GeometryDescriptor::push_color3ub(const uint8_t& r, const uint8_t& g, const uint8_t& b) {
        
        /// @brief   Check if the color format is RGB
        /// @details If the color format is not RGB, throw a runtime error
        /// @details This is a runtime safety check
        /// @warning You can remove this check for performance reasons only if you are sure that the color format is RGB
        #ifdef _ENABLE_RUNTIME_SAFETY_CHECKS_
        if(currentPrimitiveSet->get_color_format() != PrimitiveSetInstance::RGB) { throw std::runtime_error("Color format is not RGB");}
        #endif

        auto& primitiveSet = currentPrimitiveSet;
        primitiveSet->colors->push_back(r);
        primitiveSet->colors->push_back(g);
        primitiveSet->colors->push_back(b);

        /// @brief   Set the dirty flag for colors
        /// @details This is used to indicate that the colors have been modified
        /// @warning You can remove this line for performance reasons only if you are sure that you manually set the dirty flag
        #ifdef _ENABLE_AUTOMATIC_DIRTY_FLAG_MANAGEMENT_
        primitiveSet->setDirty(PrimitiveSetInstance::DIRTY_COLORS);
        #endif
    }

    /// @brief Push a color vector (r, g, b, a) to the current primitive set
    __INLINE__ void GeometryDescriptor::push_color4ub(const uint8_t& r, const uint8_t& g, const uint8_t& b , const uint8_t a) {
        
        /// @brief   Check if the color format is RGB
        /// @details If the color format is not RGB, throw a runtime error
        /// @details This is a runtime safety check
        /// @warning You can remove this check for performance reasons only if you are sure that the color format is RGB
        #ifdef _ENABLE_RUNTIME_SAFETY_CHECKS_
        if(currentPrimitiveSet->get_color_format() != PrimitiveSetInstance::RGBA) { throw std::runtime_error("Color format is not RGBA");}
        #endif

        auto& primitiveSet = currentPrimitiveSet;
        primitiveSet->colors->push_back(r);
        primitiveSet->colors->push_back(g);
        primitiveSet->colors->push_back(b);
        primitiveSet->colors->push_back(a);

        /// @brief   Set the dirty flag for colors
        /// @details This is used to indicate that the colors have been modified
        /// @warning You can remove this line for performance reasons only if you are sure that you manually set the dirty flag
        #ifdef _ENABLE_AUTOMATIC_DIRTY_FLAG_MANAGEMENT_
        primitiveSet->setDirty(PrimitiveSetInstance::DIRTY_COLORS);
        #endif
    }

    /// @brief Push indices to the current primitive set
    __INLINE__ void GeometryDescriptor::push_index(const uint32_t& index) {
        auto& primitiveSet = currentPrimitiveSet;
        primitiveSet->indices->push_back(index);

        /// @brief   Set the dirty flag for indices
        /// @details This is used to indicate that the indices have been modified
        /// @warning You can remove this line for performance reasons only if you are sure that you manually set the dirty flag
        #ifdef _ENABLE_AUTOMATIC_DIRTY_FLAG_MANAGEMENT_
        primitiveSet->setDirty(PrimitiveSetInstance::DIRTY_INDICES);
        #endif
    }

    /// @brief Push a position array to the current primitive set
    __INLINE__ void GeometryDescriptor::push_pos_array(const std::vector<float>& position_array) {

        auto& primitiveSet = currentPrimitiveSet;
        primitiveSet->positions->insert(primitiveSet->positions->end(), position_array.begin(), position_array.end());
        primitiveSet->setDirty(PrimitiveSetInstance::DIRTY_POSITIONS);     
    }

    /// @brief Push a normal array to the current primitive set
    __INLINE__ void GeometryDescriptor::push_normal_array(const std::vector<float>& normal_array) {

        auto& primitiveSet = currentPrimitiveSet;
        primitiveSet->normals->insert(primitiveSet->normals->end(), normal_array.begin(), normal_array.end());
        primitiveSet->setDirty(PrimitiveSetInstance::DIRTY_NORMALS);    
    }

    /// @brief Push a color array to the current primitive set
    __INLINE__ void GeometryDescriptor::push_color_array(const std::vector<uint8_t>& color_array) {

        auto& primitiveSet = currentPrimitiveSet;
        primitiveSet->colors->insert(primitiveSet->colors->end(), color_array.begin(), color_array.end());
        primitiveSet->setDirty(PrimitiveSetInstance::DIRTY_COLORS);

    }

    /// @brief Push a Index array to the current primitive set
    __INLINE__ void GeometryDescriptor::push_index_array(const std::vector<uint32_t>& index_array) {

        auto& primitiveSet = currentPrimitiveSet;
        primitiveSet->indices->insert(primitiveSet->indices->end(), index_array.begin(), index_array.end());
        primitiveSet->setDirty(PrimitiveSetInstance::DIRTY_INDICES);
    }

    /// @brief Copy a position array to the current primitive set
    __INLINE__ void GeometryDescriptor::copy_pos_array(const std::vector<float>& position_array) {

        auto& primitiveSet = currentPrimitiveSet;
        //primitiveSet->positions.reset();
        primitiveSet->positions = std::make_shared<std::vector<float>>(position_array);
        primitiveSet->setDirty(PrimitiveSetInstance::DIRTY_POSITIONS);
    }

    /// @brief Copy a normal array to the current primitive set
    __INLINE__ void GeometryDescriptor::copy_normal_array(const std::vector<float>& normal_array) {

        auto& primitiveSet = currentPrimitiveSet;
        //primitiveSet->normals.reset();
        primitiveSet->normals = std::make_shared<std::vector<float>>(normal_array);
        primitiveSet->setDirty(PrimitiveSetInstance::DIRTY_NORMALS);
    }

    /// @brief Copy a color array to the current primitive set
    __INLINE__ void GeometryDescriptor::copy_color_array(const std::vector<uint8_t>& color_array) {

        auto& primitiveSet = currentPrimitiveSet;
        //primitiveSet->colors.reset();
        primitiveSet->colors = std::make_shared<std::vector<uint8_t>>(color_array);
        primitiveSet->setDirty(PrimitiveSetInstance::DIRTY_COLORS);
    }

    /// @brief Copy a Index array to the current primitive set
    __INLINE__ void GeometryDescriptor::copy_index_array(const std::vector<uint32_t>& index_array) {

        auto& primitiveSet = currentPrimitiveSet;
        //primitiveSet->indices.reset();
        primitiveSet->indices = std::make_shared<std::vector<uint32_t>>(index_array);
        primitiveSet->setDirty(PrimitiveSetInstance::DIRTY_INDICES);
    }

    /// @brief Move a position array to the current primitive set
    __INLINE__ void GeometryDescriptor::move_pos_array(std::vector<float>&& position_array) {

        auto& primitiveSet = currentPrimitiveSet;
        //primitiveSet->positions.reset();
        primitiveSet->positions = std::make_shared<std::vector<float>>(std::move(position_array));
        primitiveSet->setDirty(PrimitiveSetInstance::DIRTY_POSITIONS);
    }

    /// @brief Move a normal array to the current primitive set
    __INLINE__ void GeometryDescriptor::move_normal_array(std::vector<float>&& normal_array) {

        auto& primitiveSet = currentPrimitiveSet;
        //primitiveSet->normals.reset();
        primitiveSet->normals = std::make_shared<std::vector<float>>(std::move(normal_array));
        primitiveSet->setDirty(PrimitiveSetInstance::DIRTY_NORMALS);
    }

    /// @brief Move a color array to the current primitive set
    __INLINE__ void GeometryDescriptor::move_color_array(std::vector<uint8_t>&& color_array) {

        auto& primitiveSet = currentPrimitiveSet;
        //primitiveSet->colors.reset();
        primitiveSet->colors = std::make_shared<std::vector<uint8_t>>(std::move(color_array));
        primitiveSet->setDirty(PrimitiveSetInstance::DIRTY_COLORS);
    }

    /// @brief Move a Index array to the current primitive set
    __INLINE__ void GeometryDescriptor::move_index_array(std::vector<uint32_t>&& index_array) {

        auto& primitiveSet = currentPrimitiveSet;
        //primitiveSet->indices.reset();
        primitiveSet->indices = std::make_shared<std::vector<uint32_t>>(std::move(index_array));
        primitiveSet->setDirty(PrimitiveSetInstance::DIRTY_INDICES);
    }


    /// @brief Other methods for dirty flags, etc. can be added here
    __INLINE__ void GeometryDescriptor::clearDirtyFlags() {

        for (auto& primitive : primitives) {
            primitive.second->clearDirty();
        }
    }

    /// @brief  Clear dirty flags for a specific primitive set
    /// @param name 
    __INLINE__ void GeometryDescriptor::clearDirtyFlags(const std::string& name) {

        auto it = primitives.find(name);
        if (it != primitives.end()) {
            it->second->clearDirty();
        }
    }

    /// @brief  Clear a particular dirty flag for a specific primitive set
    /// @param name 
    /// @param flag 
    __INLINE__ void GeometryDescriptor::clearDirtyFlags(const std::string& name, PrimitiveSetInstance::DirtyFlags flag) {

        auto it = primitives.find(name);
        if (it != primitives.end()) {
            it->second->clearDirty(flag);
        }
    }

    /// @brief get a primitive set by name
    /// @param name 
    /// @return std::weak_ptr<PrimitiveSetInstance>
    __INLINE__ std::weak_ptr<GeometryDescriptor::PrimitiveSetInstance> GeometryDescriptor::get_primitive_set(const std::string& name) {

        auto it = primitives.find(name);
        if (it != primitives.end()) {
            return it->second;
        }
        return std::weak_ptr<GeometryDescriptor::PrimitiveSetInstance>();
    }
    
    /// @brief get the current primitive set as a weak pointer
    /// @details lock the weak pointer to get a shared pointer
    /// @param name
    /// @return std::weak_ptr<PrimitiveSetInstance> current primitive set
    __INLINE__ std::weak_ptr<GeometryDescriptor::PrimitiveSetInstance> GeometryDescriptor::get_current_primitive_set() {

        auto it = primitives.find(currentPrimitiveSetInstanceName);
        if (it != primitives.end()) {
            return it->second;
        }
        return std::weak_ptr<GeometryDescriptor::PrimitiveSetInstance>();
    }

    /// @brief remove a primitive set by name. 
    /// @param name
    __INLINE__ void GeometryDescriptor::remove_primitive_set(const std::string& name) {
        auto it = primitives.find(name);
        if (it != primitives.end()) {
            primitives.erase(it);
        }
    }

    /// @brief remove all primitive sets
    __INLINE__ void GeometryDescriptor::remove_all_primitive_sets() {
        primitives.clear();
    }

    /// @brief copy a primitive set
    __INLINE__ void GeometryDescriptor::copy_vertex_attributes(const std::string& src, const std::string& dst) {
        auto it = primitives.find(src);
        if (it != primitives.end()) {
            if(primitives.find(dst) == primitives.end())
                primitives[dst] = std::make_shared<PrimitiveSetInstance>(dst, (it->second)->get_primitive_type_enum()); 
            *(primitives[dst]->positions) = *((it->second)->positions);
            *(primitives[dst]->normals)   = *((it->second)->normals);
            *(primitives[dst]->colors)    = *((it->second)->colors);
            *(primitives[dst]->indices)   = *((it->second)->indices);
        }
        std::string err = std::string("Primitive set not found : ") + src + std::string(" or ") + dst;
        throw std::runtime_error(err);
    }
    
    /// @brief move a primitive set
    __INLINE__ void GeometryDescriptor::move_vertex_attributes(const std::string& src, const std::string& dst) {
        auto it = primitives.find(src);
        if (it != primitives.end()) {

            primitives[dst] = std::move((it->second));
            /*
            primitives[dst]->positions = std::move((it->second)->positions);
            primitives[dst]->normals   = std::move((it->second)->normals);
            primitives[dst]->colors    = std::move((it->second)->colors);
            primitives[dst]->indices   = std::move((it->second)->indices);
            */
        }
        std::string err = std::string("Primitive set not found : ") + src + std::string(" or ") + dst;
        throw std::runtime_error(err);
    }

    /// @brief share a primitive set
    __INLINE__ void GeometryDescriptor::share_vertex_attributes(const std::string& src, const std::string& dst) {
        auto it = primitives.find(src);
        if (it != primitives.end()) {
            if(primitives.find(dst) == primitives.end())
                primitives[dst] = std::make_shared<PrimitiveSetInstance>(dst, (it->second)->get_primitive_type_enum());
            primitives[dst]->positions = ((it->second)->positions);
            primitives[dst]->normals   = ((it->second)->normals);
            primitives[dst]->colors    = ((it->second)->colors);
            primitives[dst]->indices   = ((it->second)->indices);
        }
        std::string err = std::string("Primitive set not found : ") + src + std::string(" or ") + dst;
        throw std::runtime_error(err);
    }

    /// @brief copy all primitive sets
    __INLINE__ void GeometryDescriptor::copy_all_primitive_sets(const GeometryDescriptor& src) {
        
        for(auto& primitive_set : src.primitives) {
            primitives[primitive_set.first] = std::make_shared<PrimitiveSetInstance>(primitive_set.first, primitive_set.second->get_primitive_type_enum());
            *(primitives[primitive_set.first]->positions) = *(primitive_set.second->positions);
            *(primitives[primitive_set.first]->normals)   = *(primitive_set.second->normals);
            *(primitives[primitive_set.first]->colors)    = *(primitive_set.second->colors);
            *(primitives[primitive_set.first]->indices)   = *(primitive_set.second->indices);
        }

    }
 
    /// @brief move all primitive sets
    __INLINE__ void GeometryDescriptor::move_all_primitive_sets(GeometryDescriptor&& src) {
        primitives = std::move(src.primitives);
    }

    /// @brief clear all primitive sets
    __INLINE__ void GeometryDescriptor::clear_all_primitive_sets() {
        primitives.clear();
    }

    /// @brief    copy as ref position array
    /// @details  Copy the position array as a reference to another primitive set
    /// @details  This is useful when you want to share the same position array between multiple primitive sets
    /// @param src 
    /// @param dst 
    __INLINE__ void GeometryDescriptor::share_attrib_array(const std::string& src , const std::string& dst , PrimitiveSetInstance::VertexArrayType type) {
        auto it = primitives.find(src);
        if (it != primitives.end()) {
            auto& primitiveSet = it->second;
            switch (type)
            {
            case PrimitiveSetInstance::POSITION_ARRAY:
                primitives[dst]->positions = primitiveSet->positions;
                break;
            case PrimitiveSetInstance::NORMAL_ARRAY:
                primitives[dst]->normals = primitiveSet->normals;
                break;  
            case PrimitiveSetInstance::COLOR_ARRAY:
                primitives[dst]->colors = primitiveSet->colors;
                break;  
            case PrimitiveSetInstance::INDEX_ARRAY:
                primitives[dst]->indices = primitiveSet->indices;
                break;
            }
            return;
        }        
        std::string err = std::string("No PrimitiveSet with name : [") + src + std::string("]found");
        throw std::runtime_error(err);        
    }

    /// @brief    copy as value vertex attribute array
    /// @details  Copy the position array as deep copy to another primitive set
    /// @details  This is useful when you want to share a copy of the same position array between multiple primitive sets
    /// @param src 
    /// @param dst 
    __INLINE__ void GeometryDescriptor::copy_attrib_array(const std::string& src , const std::string& dst , PrimitiveSetInstance::VertexArrayType type) {
        auto it = primitives.find(src);
        if (it != primitives.end()) {
            auto& primitiveSet = it->second;
            switch (type)
            {
            case PrimitiveSetInstance::POSITION_ARRAY:
                *(primitives[dst]->positions) = *(primitiveSet->positions);
                break;
            case PrimitiveSetInstance::NORMAL_ARRAY:
                *(primitives[dst]->normals)   = *(primitiveSet->normals);
                break;  
            case PrimitiveSetInstance::COLOR_ARRAY:
                *(primitives[dst]->colors)    = *(primitiveSet->colors);
                break;  
            case PrimitiveSetInstance::INDEX_ARRAY:
                *(primitives[dst]->indices)   = *(primitiveSet->indices);
                break;
            }
            return;
        }

        std::string err = std::string("No PrimitiveSet with name : [") + src + std::string("]found");
        throw std::runtime_error(err);   
    }

    /// @brief    copy by values vertex attrib array
    /// @details  Copy the vertex attrib array by values to all other primitive set
    /// @details  This is useful when you want to copy the same vertex attrib array between multiple primitive sets
    /// @param src , 
    /// @param dst
    __INLINE__ void  GeometryDescriptor::copy_attrib_array_with_all(const std::string& src, PrimitiveSetInstance::VertexArrayType type)
    {
        for (auto &primitive_set : primitives)
            copy_attrib_array(src, primitive_set.first, type);
    }

    /// @brief    copy by ref vertex attrib array
    /// @details  Share the vertex attrib array as a reference to all other primitive set
    /// @details  This is useful when you want to share the same vertex attrib array between multiple primitive sets
    /// @param src
    /// @param dst
    __INLINE__ void GeometryDescriptor::share_attrib_array_with_all(const std::string &src, PrimitiveSetInstance::VertexArrayType type)
    {
        for (auto &primitive_set : primitives)
            share_attrib_array(src, primitive_set.first, type);
    }

    /// @brief Share Pointer to the Positions
    __INLINE__ void GeometryDescriptor::share_positions_shared_ptr(std::shared_ptr<std::vector<float>> &in_position)
    {
        currentPrimitiveSet->share_position_shared_ptr(in_position);
    }

    /// @brief Share Pointer to the Normals
    __INLINE__ void GeometryDescriptor::share_normals_shared_ptr(std::shared_ptr<std::vector<float>> &in_normal)
    {
        currentPrimitiveSet->share_normals_shared_ptr(in_normal);
    }

    /// @brief Share Pointer to the Colors
    __INLINE__ void GeometryDescriptor::share_colors_shared_ptr(std::shared_ptr<std::vector<uint8_t>> &in_color)
    {
        currentPrimitiveSet->share_colors_shared_ptr(in_color);
    }

    /// @brief SharePointer to the Indices
    __INLINE__ void GeometryDescriptor::share_indices_shared_ptr(std::shared_ptr<std::vector<uint32_t>> &in_indices)
    {
        currentPrimitiveSet->share_indices_shared_ptr(in_indices);
    }

    /// @brief  check if the Current Primitive set is drawable
    /// @return bool
    __INLINE__ bool GeometryDescriptor::isDrawable() const
    {

        return currentPrimitiveSet->isDrawable();
    }

    /// @brief  check if the geometry has any drawable primitive sets
    /// @return bool
    __INLINE__ bool GeometryDescriptor::hasAnyDrawables() const
    {
        if (primitives.empty())
        {
            return false;
        }

        for(auto& primitive : primitives) {
            if(primitive.second->isDrawable()) {
                return true;
            }
        }

        return false;
    }

    /// @brief  check if the geometry is highlighted state
    /// @return bool
    __INLINE__ bool GeometryDescriptor::isHighlighted() const {
        return currentPrimitiveSet->isHighlighted();
    }

    __INLINE__ bool GeometryDescriptor::isValid() const {
        return currentPrimitiveSet->isValid();
    }


    /// @brief    Set the color of the current primitive set
    /// @param uint8_t r, uint8_t g, uint8_t b, uint8_t a
    __INLINE__ void GeometryDescriptor::set_fill_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
    {
        currentPrimitiveSet->color = {r, g, b, a};
    }

    /// @brief    Set the Wireframe color of the current primitive set
    /// @param uint8_t r, uint8_t g, uint8_t b, uint8_t a
    __INLINE__ void GeometryDescriptor::set_wireframe_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
    {
        currentPrimitiveSet->wireframecolor = {r, g, b, a};
    }
     
    /// @brief Set the Highlight color of the current primitive set
    /// @param uint8_t r, uint8_t g, uint8_t b, uint8_t a
    __INLINE__ void GeometryDescriptor::set_highlight_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
    {
        if(currentPrimitiveSet->isHighlighted() == false)
          currentPrimitiveSet->highlightcolor = {r, g, b, a};
        else
            currentPrimitiveSet->color = {r, g, b, a};
    } 

    /// @brief Set the Selection color of the current primitive set
    /// @param uint8_t r, uint8_t g, uint8_t b, uint8_t a
    __INLINE__ void GeometryDescriptor::set_selection_highlight_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
    {
        if(currentPrimitiveSet->isSelectionHighlighted() == false)
          currentPrimitiveSet->selection_highlight_color = {r, g, b, a};
        else
        currentPrimitiveSet->color = {r, g, b, a};
    }
    
    /// @brief Set the Custom Highlight color of the current primitive set
    /// @param uint8_t r, uint8_t g, uint8_t b, uint8_t a
    __INLINE__ void GeometryDescriptor::set_custom_highlight_color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
    {
          currentPrimitiveSet->custom_highlight_color = {r, g, b, a};
    }
    /// @brief Set the pick scheme of the current primitive set
    /// @param GLenum scheme
    __INLINE__ void GeometryDescriptor::set_pick_scheme(const GLenum scheme)
    {
        currentPrimitiveSet->set_pick_scheme(scheme);
    }

    /// @brief    Set the shading model of the current primitive set
    /// @param GLenum Shademodel
    __INLINE__ void GeometryDescriptor::set_shading_model(const GLenum model)
    {
        currentPrimitiveSet->set_shading_model(model);
    }

    /// @brief    Set the wireframe mode of the current primitive set
    /// @param GLenum Wireframemode
    __INLINE__ void GeometryDescriptor::set_wireframe_mode(const GLenum mode)
    {
        currentPrimitiveSet->set_wireframe_mode(mode);
    }
    
    /// @brief Set the Highlight mode of the current primitive set
    /// @param bool
    __INLINE__ void  GeometryDescriptor::set_hover_highlights(const bool& highlight_flag)
    {
        currentPrimitiveSet->set_hover_highlights(highlight_flag);
    }

    /// @brief Set the Highlightable mode of the current primitive set
    /// @param bool
    __INLINE__ void  GeometryDescriptor::set_hover_highlightable_mode(const bool& isHighlightable)
    {
        currentPrimitiveSet->set_hover_highlightable(isHighlightable);
    }

    /// @brief Set the Highlight mode of the current primitive set
    /// @param bool
    __INLINE__ void  GeometryDescriptor::set_selection_highlights(const bool& highlight_flag)
    {
        currentPrimitiveSet->set_selection_highlights(highlight_flag);
    }

    /// @brief Set the Selection Highlightable mode of the current primitive set
    /// @param bool
    __INLINE__ void  GeometryDescriptor::set_selection_highlightable_mode(const bool& isHighlightable)
    {
        currentPrimitiveSet->set_selection_highlightable(isHighlightable);
    }

    /// @brief Enable Custom Highlight color
    /// @param bool
    __INLINE__ void GeometryDescriptor::set_custom_highlights(const bool &enable_flag)
    {
        (*currentPrimitiveSet).is_using_custom_highlight_color = enable_flag;
    }

    /// @brief    Set the color scheme of the current primitive set
    /// @param GLenum ColorScheme
    __INLINE__ void GeometryDescriptor::set_color_scheme(const GLenum scheme)
    {
        currentPrimitiveSet->set_color_scheme(scheme);
    }

    /// @brief    Set the color format of the current primitive set
    /// @param GLenum ColorFormat
    __INLINE__ void GeometryDescriptor::set_color_format(const GLenum format)
    {
        currentPrimitiveSet->set_color_format(format);
    }

    /// @brief    Set the material property of the current primitive set
    /// @param GLenum MaterialProperty
    __INLINE__ void GeometryDescriptor::set_material_property(const GLenum property)
    {
        currentPrimitiveSet->set_material_property(property);
    }
    
    /// @brief    Set the Blend func of the current primitive set
    /// @param GLenum BlendFunc 
    __INLINE__ void GeometryDescriptor::set_blend_func(const GLenum func)
    {
        currentPrimitiveSet->set_blend_func(func);
    }

    /// @brief  Set the Point size of the current primitive set
    /// @param float PointSize
    __INLINE__ void GeometryDescriptor::set_point_size(const float& size)
    {
        currentPrimitiveSet->set_point_size(size);
    }

    /// @brief  Set the Line width of the current primitive set
    /// @param float LineWidth
    __INLINE__ void GeometryDescriptor::set_line_width(const float& width)
    {
        currentPrimitiveSet->set_line_width(width);
    }
    
    /// @brief  Update the vertex at a given index
    /// @param std::array<float, 3> translation_vector
    /// @param uint32_t index
    __INLINE__ void GeometryDescriptor::translate_vertex(const std::array<float, 3>& translation_vector, const uint32_t& index = 0xffffffff)
    {
        std::array<float, 3> position;
        float* old_pos = currentPrimitiveSet->get_vertex_ref(index);
        position[0] = old_pos[0] + translation_vector[0];
        position[1] = old_pos[1] + translation_vector[1];
        position[2] = old_pos[2] + translation_vector[2];
        currentPrimitiveSet->update_vertex(position, index);
    }

    /// @brief  Update the vertex at a given index
    /// @param std::array<float, 3> translation_vector
    /// @param uint32_t index
    __INLINE__ void GeometryDescriptor::update_vertex(const std::array<float, 3>& position, const uint32_t& index = 0xffffffff)
    {
        currentPrimitiveSet->update_vertex(position, index);
    }

    /// @brief check if the Node Manipulation is enabled
    /// @return bool
    __INLINE__ bool GeometryDescriptor::isNodeManipulationEnabled() const
    {
        return currentPrimitiveSet->isNodeManipulationEnabled();
    }

    /// @brief Set the Node Manipulation
    /// @param bool
    __INLINE__ void GeometryDescriptor::set_node_manipulator(const bool& flag)
    {
        currentPrimitiveSet->set_node_manipulator(flag);
    }

    /// @brief Get the Bounding Box of the current primitive set
    /// @return std::array<float, 6>
    __INLINE__ std::array<float, 6> GeometryDescriptor::get_bounding_box() const
    {
        return currentPrimitiveSet->get_bounding_box();
    }
}