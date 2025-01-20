
#ifndef GRIDPRO_DEBUG_MODULE_HPP
#define GRIDPRO_DEBUG_MODULE_HPP

#include <iostream>
#include <string>
#include <utility>
#include <unordered_map>
#include <fstream>

#include "nlohmann/json.hpp"

namespace gridpro_debug {

namespace category  
{
    static std::string file_path;
    static std::unordered_map<std::string, bool> debug_switches;
}

template <typename... Args>
struct TuplePrinter;

template <typename Arg>
struct TuplePrinter<Arg>
{
  inline static void print(const Arg& arg)
    {
      std::cout << arg << " ";
    }
};

template <typename Arg, typename... Args>
struct TuplePrinter<Arg, Args...>
{
 inline static void print(const Arg& arg, const Args&... args)
    {
        std::cout << arg << " ";
        TuplePrinter<Args...>::print(args...);
    }
};

template <typename... Args>
inline void console_log(const std::string& log_category, const Args&... args)
{ 
     if(category::debug_switches[log_category] == true)
     {
       std::cout << "[GP_LOG] " <<  "(" << log_category << ") "  <<  ": ";
       TuplePrinter<Args...>::print(args...);
       std::cout << "\n";
     }    
}

static void init_flags(const std::string& file_path)
{
   try
   {
    if(file_path.empty())
    {
        throw std::runtime_error("File Path is empty");
    }

    gridpro_debug::category::file_path = file_path;   
    std::ifstream inputFile(gridpro_debug::category::file_path);    
    nlohmann::json readJson;
    inputFile >> readJson;
    inputFile.close();

    // Deserialize the JSON object to another unordered_map
    category::debug_switches = readJson.get<std::unordered_map<std::string, bool>>();

   }
   catch(const std::exception& e)
   {
       //std::cerr << "Error : " << e.what() << std::endl;
   }
}

} // namespace gridpro_debug


#define GP_DEBUG_INIT(file_path)          gridpro_debug::init_flags(file_path)
#define GP_CONSOLE_LOG(log_category, ...) gridpro_debug::console_log(log_category, __VA_ARGS__)



#endif // GRIDPRO_DEBUG_MODULE_HPP