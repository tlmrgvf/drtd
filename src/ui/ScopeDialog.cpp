#include "ScopeDialog.hpp"
#include <Drtd.hpp>
#include <cassert>
#include <ui/MainGui.hpp>
#include <ui/component/Scope.hpp>
#include <util/Util.hpp>

using namespace Ui;

ScopeDialog::ScopeDialog()
    : Fl_Window(0, 0, 360, 105, "Scope settings")
    , m_zoom(80, 4, w() - 120, 25, "Zoom: ")
    , m_reset_zoom(m_zoom.x() + m_zoom.w() + 4, m_zoom.y(), 15, m_zoom.h(), "R")
    , m_remove_bias(m_zoom.x(), m_zoom.y() + m_zoom.h() + 4, 140, 25, "Remove DC-Bias")
    , m_normalized(w() - 140, m_remove_bias.y(), 100, 25, "Normalized")
    , m_pause(Util::center(w() / 2, 150), m_remove_bias.y() + m_remove_bias.h() + 5, 150, 30, "Pause")
    , m_capture(w() / 2 + Util::center(w() / 2, 150), m_pause.y(), 150, 30, "Capture") {
    icon(Drtd::drtd_icon());
    m_zoom.type(FL_HOR_NICE_SLIDER);
    m_zoom.align(FL_ALIGN_LEFT);
    m_zoom.selection_color(Util::s_amber_color);
    m_zoom.bounds(-50, 50);
    m_zoom.callback(save_to_scope);

    m_reset_zoom.callback([](Fl_Widget*, void*) {
        s_scope_dialog->m_zoom.value(0);
        save_to_scope(nullptr, nullptr);
    });

    m_pause.type(FL_TOGGLE_BUTTON);
    m_pause.selection_color(FL_RED);
    m_pause.align(FL_ALIGN_CENTER);

    m_remove_bias.callback(save_to_scope);
    m_normalized.callback(save_to_scope);
    m_pause.callback(save_to_scope);
    m_capture.callback([](Fl_Widget*, void*) { Drtd::main_gui().scope().single_shot(); });
}

void ScopeDialog::show_dialog() {
    auto& main_gui = Drtd::main_gui();
    s_scope_dialog->position(
        main_gui.x() + Util::center(main_gui.w(), s_scope_dialog->w()),
        main_gui.y() + Util::center(main_gui.h(), s_scope_dialog->h()));

    s_scope_dialog->load_from_scope();
    s_scope_dialog->set_non_modal();
    s_scope_dialog->show();
}

void ScopeDialog::load_from_scope() {
    if (!s_scope_dialog.has_instance())
        return;

    auto& settings = Drtd::main_gui().scope().settings();
    s_scope_dialog->m_zoom.value(settings.zoom);
    s_scope_dialog->m_remove_bias.value(settings.remove_dc_bias);
    s_scope_dialog->m_normalized.value(settings.normalized);
}

void ScopeDialog::save_to_scope(Fl_Widget*, void*) {
    auto& settings = Drtd::main_gui().scope().settings();

    settings.zoom = static_cast<i8>(s_scope_dialog->m_zoom.value());
    settings.remove_dc_bias = s_scope_dialog->m_remove_bias.value();
    settings.normalized = s_scope_dialog->m_normalized.value();

    Drtd::main_gui().scope().set_paused(s_scope_dialog->m_pause.value());
}
