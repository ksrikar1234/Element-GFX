#ifndef _OPENGL_2_1_FRAME_BUFFER_CLASS_H_
#define _OPENGL_2_1_FRAME_BUFFER_CLASS_H_

#include <vector>
#include <cstdint>
#include "abstract_frame_buffer.hpp"

namespace GridPro_GFX
{
namespace OpenGL_2_1 
{
    class framebuffer : public Abstract_Framebuffer
    {
    public:
        framebuffer();
        virtual ~framebuffer() override;
        virtual void update_current_frame_buffer() override;
    };
}
}

#endif