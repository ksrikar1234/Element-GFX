#ifndef _GP_CALLBACK_LIST_H
#define _GP_CALLBACK_LIST_H

#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <exception>
#include <sstream>
#include <type_traits>

namespace gp_common
{
    /// @brief class event_dispatcher
    /// Example Usage : 
    /// gp_common::event_dispatcher<void, int> hooks;
    /// hooks.add("print",  [](int x){ std::cout << "print: "  << x     << "\n"; });
    /// hooks.add("double", [](int x){ std::cout << "double: " << (x*2) << "\n"; });
    /// hooks.add("presenter", &presenter_object, &Gp_gui_presenter::function);
    /// hooks(5);
    /// hooks.remove("print");
    /// hooks(7);
    template <typename Signature>
    class event_dispatcher;

    /// @brief specialization for function types R(Args...)
    template <typename R, typename... Args>
    class event_dispatcher<R(Args...)>
    {
    public:
        using Callback = std::function<R(Args...)>;

        event_dispatcher() = default;
       ~event_dispatcher() = default;

        bool add(const std::string& name, const Callback& cb)
        {
            return m_callbacks.emplace(name, cb).second;
        }
        bool add(const std::string& name, Callback&& cb)
        {
            return m_callbacks.emplace(name, std::move(cb)).second;
        }

        template <typename Instance>
        bool add(const std::string &name,
                 Instance *instance,
                 R (Instance::*method)(Args...))
        {
            Callback cb = [instance, method](Args... args) -> R
            { return (instance->*method)(std::forward<Args>(args)...); };
            return m_callbacks.emplace(name, std::move(cb)).second;
        }

        template <typename Instance>
        bool add(const std::string &name,
                 Instance *instance,
                 R (Instance::*method)(Args...) const)
        {
            Callback cb = [instance, method](Args... args) -> R
            { return (instance->*method)(std::forward<Args>(args)...); };
            return m_callbacks.emplace(name, std::move(cb)).second;
        }

        bool remove(const std::string& name)
        {
            return m_callbacks.erase(name) > 0;
        }

        void clear() noexcept
        {
            m_callbacks.clear();
            m_errorLog.clear();
        }

        const std::vector<std::string>& getErrorLog() const noexcept
        {
            return m_errorLog;
        }

        void clearErrorLog() noexcept
        {
            m_errorLog.clear();
        }

        // void-returning
        template <typename Ret = R,
                  typename = std::enable_if_t<std::is_void_v<Ret>>>
        void invoke(Args... args)
        {
            for (auto &[name, cb] : m_callbacks)
            {
                try
                {
                    cb(std::forward<Args>(args)...);
                }
                catch (const std::exception &e)
                {
                    logException(name, e);
                }
                catch (...)
                {
                    logUnknown(name);
                }
            }
        }

        // non-void-returning
        template <typename Ret = R,
                  typename = std::enable_if_t<!std::is_void_v<Ret>>>
        std::vector<Ret> invoke(Args... args)
        {
            std::vector<Ret> results;
            results.reserve(m_callbacks.size());
            for (auto &[name, cb] : m_callbacks)
            {
                try
                {
                    results.push_back(cb(std::forward<Args>(args)...));
                }
                catch (const std::exception &e)
                {
                    logException(name, e);
                    results.push_back(R{});
                }
                catch (...)
                {
                    logUnknown(name);
                    results.push_back(R{});
                }
            }
            return results;
        }

        // operator() → forward to invoke()
        template <typename Ret = R,
                  typename = std::enable_if_t<std::is_void_v<Ret>>>
        void operator()(Args... args) { invoke(std::forward<Args>(args)...); }

        template <typename Ret = R,
                  typename = std::enable_if_t<!std::is_void_v<Ret>>>
        std::vector<Ret> operator()(Args... args) { return invoke(std::forward<Args>(args)...); }

    private:
        std::unordered_map<std::string, Callback> m_callbacks;
        std::vector<std::string> m_errorLog;

        void logException(const std::string &name, const std::exception &e)
        {
            std::ostringstream os;
            os << "Callback \"" << name << "\" threw std::exception: " << e.what();
            m_errorLog.push_back(os.str());
        }
        void logUnknown(const std::string &name)
        {
            std::ostringstream os;
            os << "Callback \"" << name << "\" threw unknown exception";
            m_errorLog.push_back(os.str());
        }
    };


    // pointer‐to‐function specialization, just inherits
    template <typename R, typename... Args>
    class event_dispatcher<R (*)(Args...)> : public event_dispatcher<R(Args...)>
    {};
}

#endif // CALLBACK_LIST_H