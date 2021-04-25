#include "IQMixer.hpp"
#include <Fl/fl_draw.H>
#include <cmath>
#include <ui/IQMixerDialog.hpp>
#include <util/Cmplx.hpp>

using namespace Dsp;

IQMixer::IQMixer(Hertz frequency)
    : RefableComponent<float, Cmplx, IQMixer>("IQ Mixer")
    , m_frequency(frequency) {
}

Size IQMixer::calculate_size() {
    return size;
}

IQMixer& Dsp::IQMixer::ref() {
    return *this;
}

Hertz IQMixer::frequency() const {
    return m_frequency;
}

void IQMixer::set_frequency(Hertz frequency) {
    if (frequency == m_frequency)
        return;

    m_frequency = frequency;
    m_phase_step = Util::two_pi_f / input_sample_rate() * static_cast<float>(m_frequency);
    Ui::IQMixerDialog::update_dialog();
}

SampleRate IQMixer::on_init(SampleRate input_sample_rate, int&) {
    m_phase_step = Util::two_pi_f / input_sample_rate * static_cast<float>(m_frequency);
    return input_sample_rate;
}

void IQMixer::show_config_dialog() {
    Ui::IQMixerDialog::show_dialog(make_ref());
}

void IQMixer::draw_at(Point p) {
    fl_circle(p.x() + size.w() / 2, p.y() + size.h() / 2, icon_radius);
    const unsigned corner_from_center = static_cast<unsigned>(std::roundf(std::sqrt(icon_radius * icon_radius / 2.f)));

    p.translate(icon_radius, icon_radius);
    fl_line(p.x(), p.y(), p.x() + corner_from_center, p.y() + corner_from_center);
    fl_line(p.x(), p.y(), p.x() + corner_from_center, p.y() - corner_from_center);
    fl_line(p.x(), p.y(), p.x() - corner_from_center, p.y() + corner_from_center);
    fl_line(p.x(), p.y(), p.x() - corner_from_center, p.y() - corner_from_center);
}

Cmplx IQMixer::process(float sample) {
    Cmplx result(sample * std::cos(m_phase), sample * std::sin(m_phase));
    m_phase += m_phase_step;
    m_phase = std::remainder(m_phase, Util::two_pi_f);
    return result;
}
