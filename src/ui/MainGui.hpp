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
    Fl_Spinner* m_frequency_spinner { nullptr };
    Fl_Tile* m_splitter_tile { nullptr };
    Scope* m_scope { nullptr };
    Waterfall* m_waterfall { nullptr };
    Fl_Group* m_content_box { nullptr };
    Fl_Box* m_status_bar { nullptr };
};

}
