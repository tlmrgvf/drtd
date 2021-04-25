#include "MainGui.hpp"
#include "ConfigDialog.hpp"
#include <Drtd.hpp>
#include <Fl/Fl_Button.H>
#include <Fl/Fl_Spinner.H>
#include <Fl/Fl_Tile.H>
#include <decoder/Decoder.hpp>
#include <ui/FirFilterDialog.hpp>
#include <ui/ScopeDialog.hpp>
#include <ui/WaterfallDialog.hpp>
#include <ui/component/Scope.hpp>
#include <ui/component/Waterfall.hpp>
#include <util/Config.hpp>
#include <util/Logger.hpp>

using namespace Ui;

static constexpr const char* s_conf_scope { "MainGui.Scope" };
static constexpr const char* s_conf_waterfall { "MainGui.GlobalWaterfall" };
static constexpr u8 header_component_height { 30 };
static constexpr u8 default_monitor_area_height { 100 };
static constexpr u8 default_scope_width { 175 };

MainGui::MainGui(u8 initial_decoder_index, WindowProperties properties)
    : Fl_Double_Window(properties.position.x(), properties.position.y(), properties.size.w(), properties.size.h(), "drtd") {
    icon(Drtd::drtd_icon());
    size_range(default_window_width, default_window_height);
    const auto width = properties.size.w();
    const auto height = properties.size.h();

    /* Header with combobox, button and spinner */
    m_header_group = new Fl_Group(2, 2, width - 4, header_bar_height);
    const auto header_component_offset = Util::center(m_header_group->h(), header_component_height);
    m_decoder_choice = new Fl_Choice(m_header_group->x() + 4,
                                     m_header_group->y() + header_component_offset,
                                     120,
                                     header_component_height);

    m_configure_button = new Fl_Button(m_decoder_choice->x() + m_decoder_choice->w() + 4,
                                       m_header_group->y() + header_component_offset,
                                       100,
                                       header_component_height,
                                       "Configure...");

    auto* header_spring = new Fl_Box(m_configure_button->x() + m_configure_button->w(), m_configure_button->y(), 1, 1);
    constexpr int spinner_width = 100;
    m_frequency_spinner = new Fl_Spinner(m_header_group->x() + m_header_group->w() - spinner_width - 4,
                                         m_header_group->y() + header_component_offset,
                                         spinner_width,
                                         header_component_height,
                                         "Center frequency:");
    m_header_group->box(FL_DOWN_BOX);

    Drtd::for_each_decoder([&](auto& decoder) { m_decoder_choice->add(decoder.name().c_str()); });
    m_decoder_choice->value(initial_decoder_index);

    m_header_group->resizable(header_spring);
    m_header_group->end();

    /* Resizable tile with content box, waterfall and scope */
    const auto tile_y = m_header_group->y() + m_header_group->h() + 2;
    m_splitter_tile = new Fl_Tile(2, tile_y, width - 4, height - tile_y - status_bar_height - 2);
    resizable(m_splitter_tile);
    const auto tile_bottom = tile_y + m_splitter_tile->h();

    m_scope = new Scope(m_splitter_tile->x(), tile_bottom - default_monitor_area_height, default_scope_width, default_monitor_area_height);
    Util::Config::load(s_conf_scope, m_scope->settings(), {});

    m_content_box = new Fl_Group(m_splitter_tile->x(), m_splitter_tile->y(), m_splitter_tile->w(), m_scope->y() - m_splitter_tile->y());
    m_content_box->box(FL_DOWN_BOX);
    m_content_box->end();

    Waterfall::GlobalSettings waterfall_settings;
    Util::Config::load(s_conf_waterfall, waterfall_settings, {});
    const auto waterfall_x = m_scope->x() + m_scope->w();
    m_waterfall = new Waterfall(waterfall_settings, waterfall_x, m_scope->y(), width - waterfall_x - 2, m_scope->h());
    m_splitter_tile->end();

    /* Register callbacks */
    callback([](auto) {
        if (Fl::event() != FL_SHORTCUT || Fl::event_key() != FL_Escape)
            Drtd::main_gui().close_all();
    });

    m_callback_manager.register_callback(*m_frequency_spinner, [&]() {
        Drtd::active_decoder()->set_center_frequency(static_cast<Hertz>(m_frequency_spinner->value()));
    });

    m_callback_manager.register_callback(*m_decoder_choice, [&]() {
        resize_monitor_area(default_monitor_area_height);
        Drtd::use_decoder(static_cast<u8>(m_decoder_choice->value()));
    });

    m_callback_manager.register_callback(*m_configure_button, [&]() {
        ConfigDialog::show_dialog();
    });

    m_callback_manager.register_callback(*m_splitter_tile, [&]() { ensure_content_box_size(); });

    /* Status bar */
    m_status_bar = new Fl_Box(m_splitter_tile->x(),
                              m_splitter_tile->y() + m_splitter_tile->h() + 2,
                              m_splitter_tile->w(),
                              status_bar_height - 4,
                              "Ready.");

    m_status_bar->box(FL_EMBOSSED_BOX);
    m_status_bar->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    m_status_bar->labelfont(FL_BOLD);
    end();
}

void MainGui::ensure_content_box_size() {
    if (m_min_content_box_height && m_content_box->h() < m_min_content_box_height)
        resize_monitor_area(std::max(m_scope->h() - (m_min_content_box_height - m_content_box->h()), 1));
}

void MainGui::set_status(const std::string& str) {
    m_status_bar->copy_label(str.c_str());
}

int Ui::MainGui::handle(int event) {
    ensure_content_box_size();
    return Fl_Double_Window::handle(event);
}

void MainGui::resize_monitor_area(u16 height) {
    m_waterfall->resize(m_waterfall->x(), m_waterfall->y() + m_waterfall->h() - height, m_waterfall->w(), height);
    m_scope->resize(m_scope->x(), m_scope->y() + m_scope->h() - height, m_scope->w(), height);
    m_content_box->size(m_content_box->w(), m_waterfall->y() - m_content_box->y());
    damage(FL_DAMAGE_ALL);
}

void MainGui::update_center_frequency() {
    m_waterfall->force_redraw();
    m_frequency_spinner->value(Drtd::active_decoder()->center_frequency());
}

void MainGui::update_decoder() {
    ConfigDialog::refresh();
    WaterfallDialog::load_from_waterfall(m_waterfall->new_settings());
    ScopeDialog::load_from_scope();

    const auto& decoder = Drtd::active_decoder();
    auto& marker = decoder->marker();

    if (marker.moveable && marker.markers.size()) {
        m_frequency_spinner->activate();
        m_frequency_spinner->range(decoder->min_center_frequency(), decoder->input_sample_rate() / 2);
        m_frequency_spinner->step(1);
    } else {
        m_frequency_spinner->deactivate();
    }

    m_waterfall->set_sample_rate(decoder->input_sample_rate());
    update_center_frequency();
}

void MainGui::close_all() {
    Drtd::stop_processing();

    Fl_Window* current = Fl::next_window(this);
    while (current) {
        auto window = current;
        current = Fl::next_window(current);
        window->hide();
    }

    Util::Config::save(s_conf_scope, m_scope->settings());
    Util::Config::save(s_conf_waterfall, m_waterfall->global_settings());
    hide();
}

void MainGui::monitor(float a) {
    m_scope->process_sample(a);
    m_waterfall->process_sample(a);
}
