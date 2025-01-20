#ifndef GP_GUI_OPENGL_3_3_TEXTURE_H
#define GP_GUI_OPENGL_3_3_TEXTURE_H

#include <string>
#include "stb_image.h"
#include "gp_gui_typedefs.h"

namespace gridpro_gui
{

    namespace OpenGL_3_3
    {
        /// @brief OpenGL Texture class
        /// The OpenGL Texture class is used to load and bind textures
        /// to the OpenGL context
        class OpenGLTexture
        {
        public:
            OpenGLTexture();
            OpenGLTexture(const std::string &imagePath);
           ~OpenGLTexture();

            void loadTex(const std::string &imagePath);
            void bind(const GLuint slot) const;
            void unbind();
            operator const GLuint() const;

        private:
            void loadTexture(const std::string &imagePath);

            GLuint textureId;
            mutable GLuint m_slot;
        };

    } // namespace OpenGL_3_3

} // namespace gridpro_gui

#endif // GP_GUI_OPENGL_3_3_TEXTURE_H
