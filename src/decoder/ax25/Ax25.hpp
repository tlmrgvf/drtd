#pragma once

#include "Packet.hpp"
#include <Fl/Fl_Text_Buffer.H>
#include <decoder/Decoder.hpp>
#include <list>
#include <sstream>
#include <ui/component/Indicator.hpp>
#include <ui/component/TextDisplay.hpp>
#include <util/BitBuffer.hpp>
#include <util/CallbackManager.hpp>

namespace Dsp {

class Ax25 final : public Decoder<bool> {
public:
    Ax25();

protected:
    virtual Fl_Widget* build_ui(Point top_left, Size ui_size) override;
    virtual Pipe::Line<float, bool> build_pipeline() override;
    virtual void process_pipeline_result(bool) override;
    virtual void on_setup() override;

private:
    static constexpr SampleRate sample_rate = 22050;
    static constexpr BaudRate baud_rate = 1200;
    static constexpr u8 headers_needed = 5;

    enum class State : u8 {
        WaitFlag,
        CountFlag,
        WaitData,
        WaitEnd
    };

    struct StateInfo {
        bool ignore_stuffed_bits { false };
        bool update_indicator { false };
        const char* label { "" };
    };

    static StateInfo info_for_state(State);
    void change_state_to(State);
    void packet_done();

    BitBuffer<Util::PushSequence::LsbPushedFirst, 8> m_in_buffer;
    BitBuffer<Util::PushSequence::LsbPushedFirst, 8> m_delay_buffer;
    BitBuffer<Util::PushSequence::LsbPushedFirst, 8> m_processed_buffer;
    unsigned m_one_count { 0 };
    unsigned m_header_count { 0 };
    State m_state { State::WaitFlag };
    std::vector<uint8_t> m_packet_buffer;
    Ui::TextDisplay* m_text_box { nullptr };
    Ui::Indicator* m_data_indicator { nullptr };
    Ui::Indicator* m_sync_indicator { nullptr };
    StateInfo m_current_state_info;
    CallbackManager m_callback_manager;
};

}
