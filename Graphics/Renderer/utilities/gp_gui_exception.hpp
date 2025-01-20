#ifndef _GP_GUI_EXCEPTION_HPP_
#define _GP_GUI_EXCEPTION_HPP_

#include <string>
#include <unordered_map>
#include <exception>

namespace gridpro_gui
{
    class gp_exception : public std::exception
    {
        public:
        gp_exception(const std::unordered_map<std::string, std::string> &error_map) : m_error_map(error_map) {}
        ~gp_exception() override {}
        const char* what() const noexcept override 
        {
            for(auto &error : m_error_map)
                m_error_message += error.first + " : " + error.second + "\n";
            
            return m_error_message.c_str();
        }

        private:
        std::unordered_map<std::string, std::string> m_error_map;  
        std::string m_error_message;
    };
}

#define GP_THROW(message)    throw gridpro_gui::gp_exception(message)
#define GP_EXCEPTION_HANDLER catch(const std::exception& ) catch(const gridpro_gui::gp_exception& in_exception) { std::cerr << "GridPro GUI Exception: " << exception.what() << std::endl; } catch(std::exception &exception) { std::cerr << "Standard Exception: " << exception.what() << std::endl; } catch(...) { std::cerr << "Unknown Exception" << std::endl; }  catch(...) { std::cerr << "Unknown Exception" << std::endl; }

#endif // _GP_GUI_EXCEPTION_HPP_