#include "CallbackManager.hpp"

using namespace Util;

CallbackManager::~CallbackManager() {
    forget_callbacks();
}

void CallbackManager::register_callback(Fl_Widget& widget, CallbackFunction callback) {
    const auto id = s_next_callback_id++;
    s_callbacks.emplace(id, std::make_unique<CallbackFunction>(callback));
    widget.callback(CallbackManager::callback, reinterpret_cast<void*>(id));
    m_registered.push_back(id);
}

void CallbackManager::callback(Fl_Widget*, void* id) {
    auto found = s_callbacks.find(reinterpret_cast<intptr_t>(id));
    if (found != s_callbacks.end())
        (*found->second)();
}

void CallbackManager::forget_callbacks() {
    for (unsigned id : m_registered)
        s_callbacks.erase(id);
    m_registered.clear();
}
