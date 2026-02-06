#pragma once

#include <iostream>
#include <utility>
#include <cstdlib>
#include <string>

#define GRIDPRO_API_DEBUG_PROFILE_ENABLE_FLAG

namespace gp_std_log
{
    inline std::string source_file()
    {
        std::string s(__FILE__);
        return s;
    }
    inline bool is_debug_flag_set(const char* in_debug_flag)
    {
        const char* debug_flag = getenv(in_debug_flag);
        if(debug_flag) return std::stoi(std::string(debug_flag)) > 0 ? true : false;
        else return false;
    }

    inline const bool& debug_flag()
    {
        static bool debug_flag = gp_std_log::is_debug_flag_set("GP_DEBUG");
        return debug_flag;
    }

    template <typename... Args>
    struct VardiacPrinter;

    template <typename Arg>
    struct VardiacPrinter<Arg>
    {
        inline static void print(const Arg& arg)
        {
            std::cout << arg << " ";
        }
    };

    template <typename Arg, typename... Args>
    struct VardiacPrinter<Arg, Args...>
    {
        inline static void print(const Arg& arg, const Args&... args)
        {
            std::cout << arg << " ";
            VardiacPrinter<Args...>::print(args...);
        }
    };

    template <typename... Args>
    inline void DEBUG_PRINT(const Args&... args)
    { 
        if(debug_flag())
        {
        std::cout << " [TRACE INFO] : ";
        VardiacPrinter<Args...>::print(args...);
        std::cout << "\n";
        }
    }

    template <typename... Args>
    inline void ERROR_PRINT(const Args&... args)
    { 
        std::cout << " [ERROR INFO] : ";
        VardiacPrinter<Args...>::print(args...);
        std::cout << "\n";
    }

    template <typename... Args>
    inline void DEBUG_PLAIN_PRINT(const Args&... args)
    {   
        if(debug_flag() == false) return;
        VardiacPrinter<Args...>::print(args...);
        std::cout << "\n";
    }

    template <typename... Args>
    inline void SIMPLE_PRINT(const Args&... args)
    {   
        VardiacPrinter<Args...>::print(args...);
        std::cout << "\n";
    }


    template <typename... Args>
    inline void LOGGER(const char* flag, const Args&... args)
    {
        if(is_debug_flag_set(flag))
        {
            std::cout << "[ " << flag << " LOG INFO] : ";
            VardiacPrinter<Args...>::print(args...);
            std::cout << "\n";
        }
    }


#ifdef GRIDPRO_API_DEBUG_PROFILE_ENABLE_FLAG

#define GP_LOG(flag, ...)   gp_std_log::LOGGER(flag, __VA_ARGS__)
#define GP_TRACE(...)       gp_std_log::DEBUG_PRINT(__VA_ARGS__)
#define GP_ERROR(...)       gp_std_log::DEBUG_PRINT(__VA_ARGS__)
#define GP_PRINT(...)       gp_std_log::SIMPLE_PRINT(__VA_ARGS__)
#define GP_PLAIN_PRINT(...) gp_std_log::DEBUG_PLAIN_PRINT(__VA_ARGS__)

#else

#define GP_LOG(flag, ...)   void(0)
#define GP_PRINT(...)       void(0)
#define GP_TRACE(...)       void(0)
#define GP_ERROR(...)       void(0)
#define GP_PLAIN_PRINT(...) void(0)


#endif

} 

/* 
    Usage:
      GP_LOG(flag, ...) : flag is the debug flag, ... is the message to be printed
      GP_LOG("GUI", "This is a GUI message");
    
    To show:
        export GUI=1
*/
//export 