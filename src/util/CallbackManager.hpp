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
#pragma once

#include "Singleton.hpp"
#include <FL/Fl_Widget.H>
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
