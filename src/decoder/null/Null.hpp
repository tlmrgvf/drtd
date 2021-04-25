#pragma once
#include <decoder/Decoder.hpp>

namespace Dsp {

class Null final : public Decoder<float> {
public:
    Null();

protected:
    virtual Fl_Widget* build_ui(Point top_left, Size ui_size) override;
    virtual Pipe::Line<float, float> build_pipeline() override;
    virtual void process_pipeline_result(float) override {};
};

}
