#ifndef _OPENGL_3_3_FRAME_BUFFER_CLASS_H_
#define _OPENGL_3_3_FRAME_BUFFER_CLASS_H_

#include <vector>
#include <cstdint>
#include "abstract_frame_buffer.hpp"

namespace gridpro_gui
{

namespace OpenGL_3_3 
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