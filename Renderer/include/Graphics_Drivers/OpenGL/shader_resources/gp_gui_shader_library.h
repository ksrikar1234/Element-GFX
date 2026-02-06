#ifndef GP_GUI_SHADER_LIBRARY_H
#define GP_GUI_SHADER_LIBRARY_H

#include <unordered_map>
#include <memory>
#include <iostream>
#include <stdexcept>
#include <type_traits>

#include "abstract_shader.hpp"

#include "gp_gui_opengl_3_3_shader.h"

namespace GridPro_GFX
{
	///////////////////////////////////////////////////////////////////////////////////////////////////
	/// @class	ShaderLibrary is Singleton for  Adding, Caching & retrieval of pre-compiled shaders
	//////////  Only Static Functions are avaliable for Simplicity and ease of use ////////////////////
    

	template <class ShaderType = OpenGL_3_3::Shader>
	class ShaderLibrary
	{
		static_assert(std::is_base_of<Abstract_Shader, ShaderType>::value, "ShaderType must be derived from Abstract_Shader");

	public:
		std::unordered_map<std::string, std::shared_ptr<ShaderType>> Avaliable_Shaders;

		static ShaderLibrary* GetLibrary()
		{
			static ShaderLibrary SingletonShaderLibrary;
			return &SingletonShaderLibrary;
		}

		static bool HasShader(const std::string &shader_name)
		{
			if (ShaderLibrary::GetLibrary()->Avaliable_Shaders.find(shader_name) != ShaderLibrary::GetLibrary()->Avaliable_Shaders.end())
				return true;
			else
				return false;
		}

		static std::shared_ptr<ShaderType> AddShader(const std::string &shader_name, const char* VertexShaderSource, const char* FragmentShaderSource)
		{
			try
			{
				if (ShaderLibrary::HasShader(shader_name))
				{
					GP_TRACE("Shader with the name [" + shader_name + "] already exists in the library");
					return GetShader(shader_name);
				}
				else
				{
					ShaderLibrary::GetLibrary()->Avaliable_Shaders[shader_name] = std::make_shared<ShaderType>(VertexShaderSource, FragmentShaderSource);
					GP_TRACE("Shader with the name [", shader_name, "] added to the library");
					return GetShader(shader_name);
				}
			}

			catch (const std::exception &e)
			{
				// Handle the exception (print an error message, log, etc.)
				std::cerr << "Exception First throwed from ShaderLibrary::AddShader() : " << e.what() << "\n";
				// We might want to rethrow the exception here if we want to propagate it further.
				throw e;
			}
		}
        
		static std::shared_ptr<ShaderType> GetShader(const std::string &shader_name)
		{
			try
			{
				typename std::unordered_map<std::string, std::shared_ptr<ShaderType>>::iterator it = ShaderLibrary::GetLibrary()->Avaliable_Shaders.find(shader_name);
				if (it != ShaderLibrary::GetLibrary()->Avaliable_Shaders.end())
				{
					ShaderLibrary::GetLibrary()->Avaliable_Shaders["ActiveShader"] = it->second;
					return (it->second);
				}
				else
					throw std::runtime_error("Failed to Retrieve a shader with name [" + shader_name + "] from the shader library");
			}
			catch (const std::exception &e)
			{
				// Handle the exception (print an error message, log, etc.)
				std::cerr << "Exception First throwed from ShaderLibrary::GetShader() : " << e.what() << "\n";
				// We might want to rethrow the exception here if we want to propagate it further.
				throw e;
			}

			return nullptr;
		}
        
		static std::shared_ptr<ShaderType> ActiveShader()
		{
			return ShaderLibrary::GetLibrary()->Avaliable_Shaders["ActiveShader"];
		}

		static void ResetShaders(uint32_t context_id)
		{
			auto ends_with = [](const std::string& str, const std::string& suffix) -> bool 
			{
                return str.size() >= suffix.size() &&
                str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
            };

 			printf("Size of the Library : %d\n", ShaderLibrary::GetLibrary()->Avaliable_Shaders.size());
			std::string context_id_str = std::string("_") + std::to_string(context_id);		
			for(auto& shader_pair : ShaderLibrary::GetLibrary()->Avaliable_Shaders)
			{
				if(ends_with(shader_pair.first, context_id_str))
				{
					ShaderLibrary::GetLibrary()->Avaliable_Shaders.erase(shader_pair.first);
				}
			}
			printf("All Shaders Cleared from the Library for Render Context : %lu\n", context_id);
		}

		static void ResetShaders()
		{
			printf("Size of the Library : %d\n", ShaderLibrary::GetLibrary()->Avaliable_Shaders.size());
			ShaderLibrary::GetLibrary()->Avaliable_Shaders.clear();
			printf("All Shaders Cleared from the Library\n");
		}

	private:
		ShaderLibrary() {}
	   ~ShaderLibrary() {}
	};
} // namespace GridPro_GFX

#endif // GP_GUI_SHADER_LIBRARY_H