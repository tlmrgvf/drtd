/*
BSD 2-Clause License

Copyright (c) 2020, Till Mayer
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
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
