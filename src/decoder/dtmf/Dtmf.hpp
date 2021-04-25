#pragma once

#include <decoder/Decoder.hpp>
#include <ui/component/Indicator.hpp>
#include <ui/component/TextDisplay.hpp>
#include <util/CallbackManager.hpp>

namespace Dsp {

class Dtmf final : public Decoder<u8> {
public:
    Dtmf();

protected:
    virtual Pipe::Line<float, u8> build_pipeline() override;
    virtual Fl_Widget* build_ui(Point, Size) override;
    virtual void process_pipeline_result(u8) override;

private:
    char m_last_symbol { '-' };
    char m_last_valid { '-' };
    Samples m_sample_count { 0 };
    Samples m_last_interruption_length { 0 };
    Samples m_samples_since_last_valid_symbol { 0 };
    Ui::TextDisplay* m_text_box { nullptr };
    Ui::Indicator* m_detect_indicator { nullptr };
    Util::CallbackManager m_callback_manager;
};

}
