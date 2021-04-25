#include "AngleDifference.hpp"
#include <Fl/fl_draw.H>
#include <cmath>
#include <util/Cmplx.hpp>

using namespace Dsp;

AngleDifference::AngleDifference()
    : ComponentBase<Cmplx, float>("Angle difference") {
}

Size AngleDifference::calculate_size() {
    return size;
}

float AngleDifference::process(Cmplx sample) {
    const float angle = sample.angle();
    const float diff = angle - m_previous_angle;
    float sign = static_cast<float>(std::copysign(1, diff));
    float abs_diff = sign * diff;
    m_previous_angle = angle;

    /*
     * If the angle difference is bigger than 180 degrees, we are either getting a signal with a frequency of more
     * than half the sample frequency, or we wrapped around. Let's ignore the first case, and just assume
     * that our input frequency will always be in a valid range 0 - (samplerate / 2)
     */
    if (abs_diff > Util::pi_f) {
        abs_diff = Util::two_pi_f - abs_diff;
        sign *= -1;
    }

    return sign * abs_diff;
}

void AngleDifference::draw_at(Point p) {
    fl_rect(p.x(), p.y(), size.w(), size.h());
    p.translate(5, 5);

    auto width = size.w() - 11;
    auto height = size.h() - 11;
    auto center_x = static_cast<unsigned>(std::sqrt(width * width * .49) / 2);
    auto center_y = static_cast<unsigned>(std::sqrt(height * height * .49) / 2);

    fl_line(p.x(), p.y() + height, p.x() + width, p.y());
    fl_line(p.x(), p.y() + height, p.x() + width, p.y() + height);
    fl_line(p.x() + center_x, p.y() + height - center_y, p.x() + static_cast<unsigned>(.7 * width), p.y() + height);
}
