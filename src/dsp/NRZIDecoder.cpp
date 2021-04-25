#include "NRZIDecoder.hpp"
#include <Fl/fl_draw.H>

static constexpr const char* nrzidecoder_xpm[] {
    "30 26 2 1",
    " 	c None",
    ".	c #000000",
    "..............................",
    ".                            .",
    ".                            .",
    ".                            .",
    ".                            .",
    ".    ....  ....              .",
    ".    .  .  .  .              .",
    ".    .  .  .  .              .",
    ".    .  .  .  .              .",
    ".    .  .  .  .              .",
    ".    .  .  .  .              .",
    ".    .  ....  ...........    .",
    ".                            .",
    ".                            .",
    ".                            .",
    ".    ..........              .",
    ".             .              .",
    ".             .              .",
    ".             .              .",
    ".             .              .",
    ".             .              .",
    ".             ...........    .",
    ".                            .",
    ".                            .",
    ".                            .",
    ".............................."
};

using namespace Dsp;

NRZIDecoder::NRZIDecoder(bool inverted)
    : ComponentBase<bool, bool>("NRZI Decoder")
    , m_inverted(inverted) {
}

Size NRZIDecoder::calculate_size() {
    int width = 0;
    int height = 0;
    fl_measure_pixmap(nrzidecoder_xpm, width, height);
    return { static_cast<unsigned>(width), static_cast<unsigned>(height) };
}

void NRZIDecoder::draw_at(Point p) {
    fl_draw_pixmap(nrzidecoder_xpm, p.x(), p.y());
}

bool NRZIDecoder::process(bool sample) {
    bool ret = (sample == m_last_sample) == m_inverted;
    m_last_sample = sample;
    return ret;
}
