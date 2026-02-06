#ifndef GRIDPRO_ABSTRACT_SHADER_HPP
#define GRIDPRO_ABSTRACT_SHADER_HPP

#include <string>
#include <unordered_map>

#include "glm/glm.hpp"

#include "gp_gui_typedefs.h"

namespace GridPro_GFX
{
    class Abstract_Shader
    {
    public:
        /// @brief	creates, loads and activates a shader program to run on GPU
        /// @param Read Abstract_Shader from c strings
        explicit Abstract_Shader(const char *VertexShaderSource, const char *FragmentShaderSource)
        {
            m_vertexShader = std::string(VertexShaderSource);
            m_fragmentShader = std::string(FragmentShaderSource);
        }
        /// @brief	destroys the shader program
        virtual ~Abstract_Shader() = default;

        /// @brief auto cast to program value
        operator GLint() const
        {
            return m_program;
        }
        /// @brief	binds the shader program
        virtual void bind() = 0;
        /// @brief	destroys the shader program
        virtual void unbind() = 0;
        /// @brief	returns the shader program id
        GLint program() const
        {
            return m_program;
        }

        /// @brief	checks validity of shader program
        virtual bool is_valid() = 0;

        /// @brief	Set uniforms of the shader program
        virtual void Set1i(const std::string &uniform_name, const float &value) = 0;
        virtual void Set1f(const std::string &uniform_name, const float &value) = 0;
        virtual void SetVec2fv(const std::string &uniform_name, const glm::vec2 &value) = 0;
        virtual void SetVec3fv(const std::string &uniform_name, const glm::vec3 &value) = 0;
        virtual void SetVec4fv(const std::string &uniform_name, const glm::vec4 &value) = 0;
        virtual void SetMat3fv(const std::string &uniform_name, const glm::mat3 &value) = 0;
        virtual void SetMat4fv(const std::string &uniform_name, const glm::mat4 &value) = 0;

        /// @brief	deletes and unlinks a GLSL-Shader-Program
        /// @note	equivalent to glUseProgram(m_program);
        virtual void delete_shader() = 0;

        /// @brief	compiles and links a GLSL-Shader-Pair
        /// @note	to activate the shader created by this use glUseProgram(m_program);
        virtual bool createShader(const std::string &vertexShader, const std::string &fragmentShader) = 0;

        /// @brief	Gets the give uniform location
        /// @param	type	std::string
        /// @param	source	a reference to the GLSL source code as std::string
        /// @return	location if succeeded, otherwhise 0

        virtual GLint GetUniformLocation(const std::string &uniform_name) const = 0;

        /// @brief	Gets the give uniform location
        /// @param	type	std::string
        /// @param	source	a reference to the GLSL source code as std::string
        /// @return	location if succeeded, otherwhise 0

        virtual GLint GetVertexAttribLocation(const std::string &attrib_name) const = 0;

    protected:
        /// @brief	compiles a shader of give type with give GLSL source code as string
        /// @param	type	GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
        /// @param	source	a reference to the GLSL source code as std::string
        /// @return	hShader if succeeded, otherwhise 0
        virtual GLint compileShader(GLint type, const std::string &source) = 0;

    protected :
        GLint m_program;
        GLint vs, fs;
        mutable std::unordered_map<std::string, int> uniformLocations;
        mutable std::unordered_map<std::string, int> vertexAttribLocations;
        std::string m_vertexShader, m_fragmentShader;
    };
}

#endif // ABSTRACT_SHADER_HPP