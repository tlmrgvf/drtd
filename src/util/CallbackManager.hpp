#pragma once

#include "Singleton.hpp"
#include <Fl/Fl_Widget.H>
#include <functional>
#include <map>
#include <memory>

namespace Util {

class CallbackManager final {
public:
    using CallbackFunction = std::function<void()>;

    CallbackManager() = default;
    ~CallbackManager();
    CallbackManager(CallbackManager&) = delete;
    CallbackManager(CallbackManager&& other) {
        std::swap(m_registered, other.m_registered);
    };

    CallbackManager& operator=(CallbackManager&& other) {
        std::swap(m_registered, other.m_registered);
        return *this;
    };

    void register_callback(Fl_Widget&, CallbackFunction);
    void forget_callbacks();

private:
    static void callback(Fl_Widget*, void*);

    static inline intptr_t s_next_callback_id { 0 };
    static inline std::map<intptr_t, std::unique_ptr<CallbackFunction>> s_callbacks;

    std::vector<intptr_t> m_registered;
};

}

using Util::CallbackManager;
