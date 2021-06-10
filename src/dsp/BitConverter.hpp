/*
BSD 2-Clause License

Copyright (c) 2020, Till Mayer
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma once

#include <pipe/Component.hpp>
#include <util/Buffer.hpp>
#include <util/RingBuffer.hpp>

namespace Dsp {

class BitConverter final : public RefableComponent<bool, bool, BitConverter> {
public:
    struct SyncInfo final {
        float samples_per_bit;
        float baud_rate;
    };

    BitConverter(float baud_rate);
    BitConverter(u16 required_sync_bits, Buffer<float> baud_rates);

    void wait_for_sync();
    void set_baud_rates(Buffer<float>);
    Buffer<float> baud_rates() const;
    float current_baud_rate() const;
    virtual Size calculate_size() override;

    std::function<void(SyncInfo)> sync_callback;

protected:
    virtual BitConverter& ref() override;
    virtual void draw_at(Point) override;
    virtual bool process(bool) override;
    virtual u16 on_init(u16, int&) override;

private:
    void recalculate_samples_per_bit();
    bool try_pop_bit_or_abort();

    static constexpr size_t buffer_size { 1024 };
    static constexpr size_t max_similar_bits { 512 };
    static constexpr float sync_bit_accuracy { .2f };

    struct ReceivedSymbol {
        bool value { false };
        Samples samples { 0 };
        u16 bit_count(float samples_per_bit);
    };

    u16 m_required_sync_bits { 0 };
    bool m_syncing { false };
    Buffer<float> m_baud_rates;
    Buffer<float> m_samples_per_bit;
    RingBuffer<bool> m_bit_buffer { buffer_size };
    bool m_last_sample { false };
    float m_current_samples_per_bit { 0 };
    u16 m_counted_sync_bits { 0 };
    ReceivedSymbol m_receiving;
    ReceivedSymbol m_received_previously;
};

}
