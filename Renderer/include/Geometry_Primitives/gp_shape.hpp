#ifndef __GP_SHAPE_HPP__
#define __GP_SHAPE_HPP__

#include <memory>
#include <string>

namespace GridPro_GFX
{
    class GeometryDescriptor;
} 

namespace gp_primitives
{
   class shape
   {
        virtual std::shared_ptr<GridPro_GFX::GeometryDescriptor> get_descriptor(std::string desc_name) const = 0;
        virtual uint8_t dim() const = 0;
   };
}

#endif