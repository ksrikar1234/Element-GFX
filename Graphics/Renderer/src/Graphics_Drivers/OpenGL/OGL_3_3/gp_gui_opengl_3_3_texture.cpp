
#include <iostream>

#include "gp_gui_opengl_3_3_texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "graphics_api.hpp"

namespace gridpro_gui
{
    namespace OpenGL_3_3
    {

        OpenGLTexture::OpenGLTexture() : textureId(0) {}

        OpenGLTexture::OpenGLTexture(const std::string &imagePath) : textureId(0)
        {
            loadTexture(imagePath);
        }

        OpenGLTexture::~OpenGLTexture()
        {
            if (textureId != 0)
            {
                printf("OpenGLTexture::~OpenGLTexture() : Deleting texture %d\n", textureId);
                RendererAPI<QGL_3_3>()->glDeleteTextures(1, &textureId);
            }
            else
            {
                printf("OpenGLTexture::~OpenGLTexture() : Texture id is zero\n");
            }
        }

        void OpenGLTexture::loadTex(const std::string &imagePath)
        {
            loadTexture(imagePath);
        }

        void OpenGLTexture::bind(const GLuint slot = 0) const
        {
            m_slot += slot;
            RendererAPI<QGL_3_3>()->glActiveTexture(GL_TEXTURE0 + slot);
            RendererAPI<QGL_3_3>()->glBindTexture(GL_TEXTURE_2D, textureId);
        }

        void OpenGLTexture::unbind()
        {
            RendererAPI<QGL_3_3>()->glActiveTexture(GL_TEXTURE0);
            RendererAPI<QGL_3_3>()->glBindTexture(GL_TEXTURE_2D, 0);
        }

        OpenGLTexture::operator const GLuint() const
        {
            return m_slot;
        }

        void OpenGLTexture::loadTexture(const std::string &imagePath)
        {
            int width, height, channels;
            stbi_set_flip_vertically_on_load(true); // Flip image vertically because OpenGL has the origin at the bottom-left
            unsigned char *data = stbi_load(imagePath.c_str(), &width, &height, &channels, 0);
            std::cout << "channels = " << channels << std::endl;

            GLenum format;
            if (channels == 1)
            {
                format = GL_RED;
            }
            else if (channels == 3)
            {
                format = GL_RGB;
            }
            else if (channels == 4)
            {
                format = GL_RGBA;
            }
            else
            {
                format = GL_RGB;
            }

            if (data != nullptr)
            {
                // Generate and set formats

                RendererAPI<QGL_3_3>()->glGenTextures(1, &textureId);
                // RendererAPI<QGL_3_3>()->glActiveTexture(GL_TEXTURE0);
                RendererAPI<QGL_3_3>()->glBindTexture(GL_TEXTURE_2D, textureId);
                // RendererAPI<QGL_3_3>()->glTexStorage2D(textureId, 1, GL_RGBA, width, height);

                // Set texture parameters (optional)
                RendererAPI<QGL_3_3>()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                RendererAPI<QGL_3_3>()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                RendererAPI<QGL_3_3>()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                RendererAPI<QGL_3_3>()->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                // Generate the texture
                RendererAPI<QGL_3_3>()->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                RendererAPI<QGL_3_3>()->glGenerateMipmap(GL_TEXTURE_2D);

                stbi_image_free(data);

                GLenum error = RendererAPI<QGL_3_3>()->glGetError();

                if (error != GL_NO_ERROR)
                {
                    std::cerr << "OpenGL error in loadTexture: " << error << std::endl;

                    switch (error)
                    {
                    case GL_INVALID_ENUM:
                        std::cerr << "GL_INVALID_ENUM" << std::endl;
                        break;
                    case GL_INVALID_VALUE:
                        std::cerr << "GL_INVALID_VALUE" << std::endl;
                        break;
                    case GL_INVALID_OPERATION:
                        std::cerr << "GL_INVALID_OPERATION" << std::endl;
                        break;
                    case GL_INVALID_FRAMEBUFFER_OPERATION:
                        std::cerr << "GL_INVALID_FRAMEBUFFER_OPERATION" << std::endl;
                        break;
                    case GL_OUT_OF_MEMORY:
                        std::cerr << "GL_OUT_OF_MEMORY" << std::endl;
                        break;
                    default:
                        std::cerr << "Unknown error" << std::endl;
                        break;
                    }
                }
            }
            else
            {
                throw std::runtime_error("Failed to load texture: " + imagePath);
                std::cerr << "Failed to load texture: " << imagePath << std::endl;
            }
        }
        
    } // namespace OpenGL_3_3
} // namespace gridpro_gui