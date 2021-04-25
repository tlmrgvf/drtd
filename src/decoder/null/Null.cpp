#include "Null.hpp"
#include <dsp/Nothing.hpp>

using namespace Dsp;

Null::Null()
    : Decoder<float>("Null", 44100, DecoderBase::Headless::Yes, 80) {
}

Pipe::Line<float, float> Null::build_pipeline() {
    return Pipe::line(Nothing<float>());
}

Fl_Widget* Null::build_ui(Point top_left, Size ui_size) {
    auto* label = new Fl_Box(top_left.x(), top_left.y(), ui_size.w(), ui_size.h(), "Choose a decoder");
    label->align(FL_ALIGN_CENTER);
    label->labelsize(50);
    label->labelfont(FL_BOLD);
    return label;
}
