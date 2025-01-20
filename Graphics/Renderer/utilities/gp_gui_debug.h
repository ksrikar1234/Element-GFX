#ifndef _GRIDPRO_API_DEBUG_FUNCTIONS_HPP_
#define _GRIDPRO_API_DEBUG_FUNCTIONS_HPP_

#include <iostream>
#include <utility>
#include <cstdlib>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#define GRIDPRO_API_DEBUG_PROFILE_ENABLE_FLAG

enum GP_COLOR : size_t
{
    RED,
    GREEN,
    BLUE,
    YELLOW,
    CYAN,
    MAGENTA,
    WHITE,
    BRIGHT_RED,
    BRIGHT_GREEN,
    BRIGHT_YELLOW,
    BRIGHT_BLUE,
    BRIGHT_MAGENTA,
    RESET
};

#ifdef _WIN32
typedef WORD PLATFORM_COLOR_TYPE;
#else
typedef const char* PLATFORM_COLOR_TYPE;
#endif


namespace gp_std
{
    inline bool is_debug_flag_set(const char* in_debug_flag)
    {
        const char* debug_flag = getenv(in_debug_flag);
        if(debug_flag) return std::stoi(std::string(debug_flag)) > 0 ? true : false;
        else return false;
    }

    inline const bool& debug_flag()
    {
        static bool debug_flag = gp_std::is_debug_flag_set("GP_DEBUG");
        return debug_flag;
    }

    template <GP_COLOR color>
    constexpr PLATFORM_COLOR_TYPE get_color_code()
    {
#ifndef _WIN32
        // ANSI escape codes for colors
        if constexpr (color == RED)
            return "\033[31m";
        else if constexpr (color == GREEN)
            return "\033[32m";
        else if constexpr (color == BLUE)
            return "\033[34m";
        else if constexpr (color == YELLOW)
            return "\033[33m";
        else if constexpr (color == CYAN)
            return "\033[36m";
        else if constexpr (color == MAGENTA)
            return "\033[35m";
        else if constexpr (color == WHITE)
            return "\033[37m";
        else if constexpr (color == BRIGHT_RED)
            return "\033[91m";
        else if constexpr (color == BRIGHT_GREEN)
            return "\033[92m";
        else if constexpr (color == BRIGHT_YELLOW)
            return "\033[93m";
        else if constexpr (color == BRIGHT_BLUE)
            return "\033[94m";
        else if constexpr (color == BRIGHT_MAGENTA)
            return "\033[95m";
        else
            return "\033[0m"; // RESET
#else

        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        WORD consoleColor;

        switch (color) {
        case RED: consoleColor = FOREGROUND_RED | FOREGROUND_INTENSITY; break;
        case GREEN: consoleColor = FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
        case BLUE: consoleColor = FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
        case YELLOW: consoleColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
        case CYAN: consoleColor = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
        case MAGENTA: consoleColor = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
        case WHITE: consoleColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; break;
        case RESET: consoleColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE; break;
        case BRIGHT_RED: consoleColor = FOREGROUND_RED | FOREGROUND_INTENSITY; break;
        case BRIGHT_GREEN: consoleColor = FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
        case BRIGHT_YELLOW: consoleColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY; break;
        case BRIGHT_BLUE: consoleColor = FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
        case BRIGHT_MAGENTA: consoleColor = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY; break;
        }

        return consoleColor;

#endif
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
    inline void print(const Args&... args)
    { 
        VardiacPrinter<Args...>::print(args...);
    }

    // General colored print function
    template <GP_COLOR color, typename... Args>
    inline void color_print(const Args &...args)
    {
        auto colorCode = get_color_code<color>();

#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, colorCode);
        VardiacPrinter<Args...>::print(args...);
        SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // Reset
#else
        std::cout << colorCode;
        VardiacPrinter<Args...>::print(args...);
        std::cout << "\033[0m"; // Reset
#endif
    }

    template <typename... Args>
    inline void DEBUG_PRINT(const Args&... args)
    { 
        if(debug_flag())
        {
            color_print<BRIGHT_BLUE>(" [TRACE INFO] : ");
            color_print<CYAN>(args...);
            std::cout << "\n";
        }
    }

    template <typename... Args>
    inline void ERROR_PRINT(const Args&... args)
    { 
        color_print<RED>("[ERROR INFO] :");
        color_print<BRIGHT_RED>(args...);
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

    // Custom-colored SIMPLE_PRINT
    template <GP_COLOR color, typename... Args>
    inline void SIMPLE_PRINT(const Args &...args)
    {
        color_print<color>(args...);
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

#define GP_LOG(flag, ...)                gp_std::LOGGER(flag, __VA_ARGS__)
#define GP_TRACE(...)                    gp_std::DEBUG_PRINT(__VA_ARGS__)
#define GP_ERROR(...)                    gp_std::ERROR_PRINT(__VA_ARGS__)
#define GP_PRINT(...)                    gp_std::SIMPLE_PRINT(__VA_ARGS__)
#define GP_COLOR_PRINT(color_enum, ...)  gp_std::SIMPLE_PRINT<color_enum>(__VA_ARGS__)
#define GP_PLAIN_PRINT(...)              gp_std::DEBUG_PLAIN_PRINT(__VA_ARGS__)

#else

#define GP_LOG(flag, ...)   void(0)
#define GP_PRINT(...)       void(0)
#define GP_TRACE(...)       void(0)
#define GP_ERROR(...)       void(0)
#define GP_COLOR_PRINT(color_enum, ...) void(0) 
#define GP_PLAIN_PRINT(...) void(0)


#endif

} // namespace HELIUM_API
#endif // #define _GRIDPRO_API_DEBUG_FUNCTIONS_HPP_


/* 
    Usage:
      GP_LOG(flag, ...) : flag is the debug flag, ... is the message to be printed
      GP_LOG("GUI", "This is a GUI message");
    
    To show:
        export GUI=1
*/
//export 