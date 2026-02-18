#include "gp_gui_opengl_3_3_shader.h" 

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "graphics_api.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// public implementations: ////////////////////////////////////////


namespace GridPro_GFX {

namespace OpenGL_3_3
{
	
	Shader::Shader(const char* VertexShaderSource , const char* FragmentShaderSource) : Abstract_Shader(VertexShaderSource, FragmentShaderSource)
	{
		// read compile link and load a GLSL shader as a program
		try
		{
		if(!createShader(m_vertexShader, m_fragmentShader))
			throw std::runtime_error("Failed to Create the Shaders from the strings " + std::string(VertexShaderSource) + ", " + std::string(FragmentShaderSource));
		}

		catch(const std::exception& e)
		{   
			// Handle the exception (print an error message, log, etc.)
			GP_TRACE("Exception in Shader(const char* VertexShaderSource , const char* FragmentShaderSource): ", e.what());
			// We might want to rethrow the exception here if we want to propagate it further.
			throw e;
		}
		
		RendererAPI<QGL_3_3>()->glUseProgram(m_program);
	}


	Shader::Shader(const Shader& other) : Abstract_Shader(other.m_vertexShader.c_str(), other.m_fragmentShader.c_str())
	{
		m_program = other.m_program;
		vs = other.vs;
		fs = other.fs;
		uniformLocations = (other.uniformLocations);
		vertexAttribLocations = (other.vertexAttribLocations);
		m_vertexShader = (other.m_vertexShader);
		m_fragmentShader = (other.m_fragmentShader);
	}

	 
	Shader& Shader::operator=(Shader&& other) noexcept
	{
		if (this != &other) {
			// Move the resources
			m_program = other.m_program;
			vs = other.vs;
			fs = other.fs;
			uniformLocations = std::move(other.uniformLocations);
			vertexAttribLocations = std::move(other.vertexAttribLocations);
			m_vertexShader = std::move(other.m_vertexShader);
			m_fragmentShader = std::move(other.m_fragmentShader);

			// Reset the resources in the other object
			other.m_program = 0;
			other.vs = 0;
			other.fs = 0;
			}
		return *this;
	}


	void Shader::bind()
	{
		try
		{ 
			if(m_program)
			   RendererAPI<QGL_3_3>()->glUseProgram(m_program);
			else
			   throw std::runtime_error("Unable to bind Shader");
		}
		catch(const std::exception& e)
		{
			throw e;
		}
	}

	 
	void Shader::unbind()
	{  
		try
		{ 
			if(m_program != 0)
		          RendererAPI<QGL_3_3>()->glUseProgram(0);
			else
			  throw std::runtime_error("Unable to unbind Shader");
		}
		catch(const std::exception& e)
		{
			throw e;
		}	 
	}

	
	GLint Shader::GetUniformLocation(const std::string& uniform_name) const
	{
		try 
		{
			if(uniformLocations.find(uniform_name) == uniformLocations.end())
				uniformLocations[uniform_name] = RendererAPI<QGL_3_3>()->glGetUniformLocation(this->m_program, uniform_name.c_str());
				
			if(uniformLocations[uniform_name] == -1)  
				throw std::runtime_error("Failed to get uniform location for: " + uniform_name);
			
			return uniformLocations[uniform_name];	 
		}
		catch(const std::exception& e) {
				// Handle the exception (print an error message, log, etc.)
				GP_TRACE("Exception in GetUniformLocation: ", e.what());
				std::cout << "Exception in GetUniformLocation: " << e.what() << "\n";
				// We might want to rethrow the exception here if we want to propagate it further.
				throw e;
		}
		return -1;
	}

	
	GLint Shader::GetVertexAttribLocation(const std::string& attrib_name) const
	{
		try
		{
			if(vertexAttribLocations.find(attrib_name) == vertexAttribLocations.end())
				vertexAttribLocations[attrib_name] =  RendererAPI<QGL_3_3>()->glGetAttribLocation(this->m_program, attrib_name.c_str());
			
			if( vertexAttribLocations[attrib_name] == -1)  
				throw std::runtime_error("Failed to get attrib location for: " + attrib_name);
			
			return  vertexAttribLocations[attrib_name];
		}

		catch(const std::exception& e)
		{
				// Handle the exception (print an error message, log, etc.)
				GP_TRACE( "Exception in GetVertexAttribLocation: ", e.what());
				// We might want to rethrow the exception here if we want to propagate it further.
				throw e;
		}
		return -1;
	}


	 
	void Shader::Set1i(const std::string& uniform_name, const float& value)
	{
		RendererAPI<QGL_3_3>()->glUniform1i(GetUniformLocation(uniform_name) , value);
	}

	 
	void Shader::Set1f(const std::string& uniform_name, const float& value)
	{
		RendererAPI<QGL_3_3>()->glUniform1f(GetUniformLocation(uniform_name) , value);
	}

	 
	void Shader::SetVec2fv(const std::string& uniform_name, const glm::vec2& value)
	{
		RendererAPI<QGL_3_3>()->glUniform2fv(GetUniformLocation(uniform_name), 1,  glm::value_ptr(value));
	}

	 
	void Shader::SetVec3fv(const std::string& uniform_name, const glm::vec3& value)
	{
		RendererAPI<QGL_3_3>()->glUniform3fv(GetUniformLocation(uniform_name), 1,  glm::value_ptr(value));
	}

	 
	void Shader::SetVec4fv(const std::string& uniform_name, const glm::vec4& value)
	{	 
       RendererAPI<QGL_3_3>()->glUniform4fv(GetUniformLocation(uniform_name), 1,  glm::value_ptr(value)); 
	}

	 
	void Shader::SetMat3fv(const std::string& uniform_name, const glm::mat3& value)
	{	
		RendererAPI<QGL_3_3>()->glUniformMatrix3fv(GetUniformLocation(uniform_name), 1, GL_FALSE, glm::value_ptr(value));
	}

	 
	void Shader::SetMat4fv(const std::string& uniform_name, const glm::mat4& value)
	{
		RendererAPI<QGL_3_3>()->glUniformMatrix4fv(GetUniformLocation(uniform_name), 1, GL_FALSE, glm::value_ptr(value));
	}
	
	bool Shader::is_valid()
	{
		return (m_program != 0 && vs != 0 && fs != 0 && m_vertexShader.size() > 0 && m_fragmentShader.size() > 0);
	}

	void Shader::delete_shader()
	{
		try
		{
			if (m_program != 0)
			{
				RendererAPI<QGL_3_3>()->glDeleteProgram(m_program);
			}
			else
			{
				throw std::runtime_error("Falied to delete the shader program");
			}
		}
		catch(const std::exception& e)
		{   
			// Handle the exception (print an error message, log, etc.)
			GP_TRACE("Exception in ~Shader(): ", e.what());
			// We might want to rethrow the exception here if we want to propagate it further.
			throw e;
		}

		m_vertexShader.clear();
		m_fragmentShader.clear();
	}

	 
	Shader::~Shader()
	{  
		GP_PRINT("Shader ID :",  m_program , " Destructor Called");	
		delete_shader();
	}

	 Shader::operator GLint() const
	{
	   return m_program;
	}

	 
	int Shader::program()
	{
	    return this->m_program;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////// private implementations: ///////////////////////////////////////
	 
	GLint Shader::compileShader(GLint type, const std::string& source)
	{
		// creates an empty shader obj, ready to accept source-code and be compiled
		GLint hShader =  RendererAPI<QGL_3_3>()->glCreateShader(type);

		// hands the shader source code to the shader object so that it can keep a copy of it
		const char* src = source.c_str();
		RendererAPI<QGL_3_3>()->glShaderSource(hShader, 1, &src, nullptr);

		// compiles whatever source code is contained in the shader object
		RendererAPI<QGL_3_3>()->glCompileShader(hShader);

		// Error Handling: Check whether the shader has been compiled
		GLint result;
		RendererAPI<QGL_3_3>()->glGetShaderiv(hShader, GL_COMPILE_STATUS, &result);	// assigns result with compile operation status
		if (result == GL_FALSE)
		{
			int length;
			RendererAPI<QGL_3_3>()->glGetShaderiv(hShader, GL_INFO_LOG_LENGTH, &length); // assigns length with length of information log
			char* infoLog = (char*)alloca(length * sizeof(char));	// allocate on stack frame of caller
			RendererAPI<QGL_3_3>()->glGetShaderInfoLog(hShader, length, &length, infoLog);	// returns the information log for a shader object
			std::cout << "Failed to compile shader ! "
				<< (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
				<< "\n";
			std::cout << infoLog << std::endl;
			RendererAPI<QGL_3_3>()->glDeleteShader(hShader);
			return 0;
		}

		GP_TRACE((type == GL_VERTEX_SHADER ? "Vertex" : "Fragment"), "Shader Compiled Successfully");

		return hShader;
	}

	 
	bool Shader::createShader(const std::string& vertexShader, const std::string& fragmentShader) {
		// compile the two shaders given as string reference
		vs = compileShader(GL_VERTEX_SHADER, vertexShader);
		fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

		// create a container for the program-object to which you can attach shader objects
		m_program =  RendererAPI<QGL_3_3>()->glCreateProgram();

		// attaches the shader objects to the program object
		RendererAPI<QGL_3_3>()->glAttachShader(m_program, vs);
		RendererAPI<QGL_3_3>()->glAttachShader(m_program, fs);

		// links all the shader objects, that are attached to a program object, together
		RendererAPI<QGL_3_3>()->glLinkProgram(m_program);

		// Error Handling: Check whether program has been linked successfully
		GLint result;
		RendererAPI<QGL_3_3>()->glGetProgramiv(m_program, GL_LINK_STATUS, &result);	// assigns result with compile operation status
		if (result == GL_FALSE)
		{
			int length;
			RendererAPI<QGL_3_3>()->glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &length); // assigns length with length of information log
			char* infoLog =  new char[length * sizeof(char)];	// allocate on stack frame of caller
			RendererAPI<QGL_3_3>()->glGetProgramInfoLog(m_program, length, &length, infoLog);	// returns the information log for a shader object
			std::cout << "Failed to link vertex and fragment shader!" << "\n";
			std::cout << infoLog << std::endl;
			RendererAPI<QGL_3_3>()->glDeleteProgram(m_program);
			delete [] infoLog;
			return false;
		}
		RendererAPI<QGL_3_3>()->glValidateProgram(m_program);
		
		GP_TRACE("Shader Program Linked Successfully");

		// deletes intermediate objects
		RendererAPI<QGL_3_3>()->glDeleteShader(vs);
		RendererAPI<QGL_3_3>()->glDeleteShader(fs);

		// activate the program into the state machine of opengl
		// glUseProgram(m_program);

		return true;
	}
} // namespace OpenGL_3_3

} // namespace GridPro_GFX



