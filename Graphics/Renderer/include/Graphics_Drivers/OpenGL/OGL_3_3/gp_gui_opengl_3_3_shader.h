#ifndef GP_GUI_SHADER_H
#define GP_GUI_SHADER_H


#include <iostream>
#include "gp_gui_typedefs.h"
#include "gp_gui_debug.h"

#include "abstract_shader.hpp"
#include <memory>

namespace gridpro_gui
{

namespace OpenGL_3_3
{
	class Shader : public Abstract_Shader
	{
	public:
		/// @brief Read and construct  Shader from c strings
		Shader(const char *VertexShaderSource, const char *FragmentShaderSource);
		/// @brief	destroys the shader program
		virtual ~Shader() override;

		/// @brief	compiles and links a GLSL-Shader-Pair
		/// @note	to activate the shader created by this use glUseProgram(m_program);
		bool createShader(const std::string &vertexShader, const std::string &fragmentShader);

		/// @brief	deletes and unlinks a GLSL-Shader-Program
		/// @note	equivalent to glUseProgram(m_program);
		void delete_shader();

		/// @brief	binds the shader program
		void bind();
		/// @brief	destroys the shader program
		void unbind();
		/// @brief	checks validity of shader program
		bool is_valid();

		/// @brief auto cast to program value
		operator GLint() const;
		/// @brief	returns the shader program id
		GLint program();

		/// @brief	Set uniforms of the shader program
		void Set1i(const std::string &uniform_name, const float &value);
		void Set1f(const std::string &uniform_name, const float &value);
		void SetVec2fv(const std::string &uniform_name, const glm::vec2 &value);
		void SetVec3fv(const std::string &uniform_name, const glm::vec3 &value);
		void SetVec4fv(const std::string &uniform_name, const glm::vec4 &value);
		void SetMat3fv(const std::string &uniform_name, const glm::mat3 &value);
		void SetMat4fv(const std::string &uniform_name, const glm::mat4 &value);

		/// @brief	Gets the give uniform location
		/// @param	type	std::string
		/// @param	source	a reference to the GLSL source code as std::string
		/// @return	location if succeeded, otherwhise 0

		GLint GetUniformLocation(const std::string &uniform_name) const;

		/// @brief	Gets the give uniform location
		/// @param	type	std::string
		/// @param	source	a reference to the GLSL source code as std::string
		/// @return	location if succeeded, otherwhise 0

		GLint GetVertexAttribLocation(const std::string &attrib_name) const;

		// Move assignment operator
		Shader &operator=(const Shader &other) noexcept;
		// Move assignment operator
		Shader &operator=(Shader &&other) noexcept;
		// Copy constructor
		Shader(const Shader &other);

	private:
		/// @brief	compiles a shader of give type with give GLSL source code as string
		/// @param	type	GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
		/// @param	source	a reference to the GLSL source code as std::string
		/// @return	hShader if succeeded, otherwhise 0
		GLint compileShader(GLint type, const std::string &source);
	};
} // namespace OpenGL_3_3
} // namespace gridpro_gui

#endif // GP_GUI_SHADER_H
