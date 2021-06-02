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

#include <Fl/Fl_Double_Window.H>
#include <util/CallbackManager.hpp>
#include <util/Point.hpp>
#include <util/Size.hpp>
#include <util/Types.hpp>

class Fl_Button;
class Fl_Spinner;
class Fl_Tile;
class Fl_Choice;
class Fl_Box;

namespace Ui {

class Scope;
class Waterfall;

class MainGui final : public Fl_Double_Window {
public:
    static constexpr u16 default_window_width { 640 };
    static constexpr u16 default_window_height { 480 };
    static constexpr u8 header_bar_height { 45 };
    static constexpr u8 status_bar_height { 30 };
    static constexpr u16 window_padding { header_bar_height + status_bar_height + 10 };

    struct WindowProperties {
        Point position;
        Size size;
    };

    MainGui(u8 initial_decoder, WindowProperties properties);
    void monitor(float);
    void update_center_frequency();
    void update_decoder();
    void update_snr(float);
    void hide_snr();

    Scope& scope() const { return *m_scope; }
    Waterfall& waterfall() const { return *m_waterfall; }
    Fl_Group& content_box() { return *m_content_box; };
    void set_status(const std::string&);
    void set_min_content_box_height(u16 height) {
        m_min_content_box_height = height;
        ensure_content_box_size();
    }
    virtual int handle(int) override;

private:
    void close_all();
    void ensure_content_box_size();
    void resize_monitor_area(u16 height);

    u16 m_min_content_box_height { 0 };
    CallbackManager m_callback_manager;
    Fl_Group* m_header_group { nullptr };
    Fl_Choice* m_decoder_choice { nullptr };
    Fl_Button* m_configure_button { nullptr };
    Fl_Box* m_snr { nullptr };
    Fl_Spinner* m_frequency_spinner { nullptr };
    Fl_Tile* m_splitter_tile { nullptr };
    Scope* m_scope { nullptr };
    Waterfall* m_waterfall { nullptr };
    Fl_Group* m_content_box { nullptr };
    Fl_Box* m_status_bar { nullptr };
};

}
