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
#include "BitConverter.hpp"
#include <Fl/fl_draw.H>
#include <cmath>

static constexpr const char* bitconverter_xpm[] {
    "40 30 2 1",
    " 	c None",
    ".	c #000000",
    "........................................",
    ".                                      .",
    ".                                      .",
    ".                                      .",
    ".                                      .",
    ".                                      .",
    ".     ....                    ....     .",
    ".        .                    .        .",
    ".        .         ..         .        .",
    ".        .        .  .        .        .",
    ".        .        .  .        .        .",
    ".        .        .  .        .        .",
    ".        .         ..         .        .",
    ".        .   .            .   .        .",
    ".        .  .              .  .        .",
    ".        . .................. .        .",
    ".        .  .              .  .        .",
    ".        .   .            .   .        .",
    ".        .                    .        .",
    ".        .                    .        .",
    ".        .                    .        .",
    ".        .                    .        .",
    ".        .                    .        .",
    ".        ......................        .",
    ".                                      .",
    ".                                      .",
    ".                                      .",
    ".                                      .",
    ".                                      .",
    "........................................"
};

using namespace Dsp;

BitConverter::BitConverter(float baud_rate)
    : BitConverter(0, { baud_rate }) {
}

BitConverter::BitConverter(u16 required_sync_bits, Buffer<float> baud_rates)
    : RefableComponent<bool, bool, BitConverter>("Bit converter")
    , m_required_sync_bits(required_sync_bits)
    , m_baud_rates(std::move(baud_rates)) {
}

SampleRate BitConverter::on_init(SampleRate sample_rate, int&) {
    recalculate_samples_per_bit();
    return sample_rate;
}

Buffer<float> BitConverter::baud_rates() const {
    return m_baud_rates;
}

void BitConverter::wait_for_sync() {
    m_syncing = true;
    m_current_samples_per_bit = 0;
    m_receiving = {};
    m_received_previously = {};
    m_bit_buffer.clear();
}

void BitConverter::set_baud_rates(Buffer<float> rates) {
    assert(rates.size());
    m_baud_rates = std::move(rates);
    recalculate_samples_per_bit();
}

BitConverter& Dsp::BitConverter::ref() {
    return *this;
}

void BitConverter::recalculate_samples_per_bit() {
    assert(m_baud_rates.size());
    m_samples_per_bit = Buffer<float>(m_baud_rates.size());

    for (size_t i = 0; i < m_baud_rates.size(); ++i)
        m_samples_per_bit[i] = input_sample_rate() / m_baud_rates[i];

    if (m_samples_per_bit.size() == 1) {
        m_current_samples_per_bit = m_samples_per_bit[0];
        m_required_sync_bits = 0;
        m_syncing = false;
    } else {
        m_current_samples_per_bit = 0;
        m_syncing = true;
    }
}

bool BitConverter::try_pop_bit_or_abort() {
    if (m_bit_buffer.count())
        return m_bit_buffer.pop();

    GenericComponent::abort_processing();
    return false;
}

Size BitConverter::calculate_size() {
    int width = 0;
    int height = 0;
    fl_measure_pixmap(bitconverter_xpm, width, height);
    return { static_cast<unsigned>(width), static_cast<unsigned>(height) };
}

void BitConverter::draw_at(Point p) {
    fl_draw_pixmap(bitconverter_xpm, p.x(), p.y());
}

float BitConverter::current_baud_rate() const {
    return input_sample_rate() / m_current_samples_per_bit;
}

bool BitConverter::process(bool sample) {
    if (sample == m_last_sample) {
        ++m_receiving.samples;
        return try_pop_bit_or_abort();
    }

    m_receiving.value = m_last_sample;
    m_last_sample = sample;

    if (m_syncing) {
        if (m_current_samples_per_bit == 0) {
            for (auto samples : m_samples_per_bit) {
                if (std::abs(1 - static_cast<float>(m_receiving.samples) / samples) <= sync_bit_accuracy) {
                    m_current_samples_per_bit = samples;
                    m_counted_sync_bits = 1;
                    break;
                }
            }
        } else {
            auto count = m_receiving.bit_count(m_current_samples_per_bit);
            if (count == 1) {
                ++m_counted_sync_bits;
                if (m_counted_sync_bits == m_required_sync_bits) {
                    m_counted_sync_bits = 0;
                    m_syncing = false;
                    float bauds = current_baud_rate();
                    logger().info() << "Synced to " << bauds << " bauds";
                    if (sync_callback)
                        sync_callback({ m_current_samples_per_bit, bauds });
                }
            } else {
                m_current_samples_per_bit = 0;
                m_counted_sync_bits = 0;
            }
        }

        m_receiving = ReceivedSymbol();
        return try_pop_bit_or_abort();
    }

    if (m_current_samples_per_bit == 0)
        return try_pop_bit_or_abort();

    if (m_bit_buffer.count() == buffer_size) {
        logger().warning() << "Buffer full!";
        return try_pop_bit_or_abort();
    }

    auto count = m_receiving.bit_count(m_current_samples_per_bit);
    if (!count) {
        m_received_previously.samples += m_receiving.samples;
        m_receiving = ReceivedSymbol();
        return try_pop_bit_or_abort();
    } else if (count >= max_similar_bits) {
        m_receiving = ReceivedSymbol();
        return try_pop_bit_or_abort();
    }

    for (u16 i = 0; i < m_received_previously.bit_count(m_current_samples_per_bit); ++i)
        m_bit_buffer.push(m_received_previously.value);

    m_received_previously = std::move(m_receiving);
    m_receiving = ReceivedSymbol();

    return try_pop_bit_or_abort();
}

u16 BitConverter::ReceivedSymbol::bit_count(float samples_per_bit) {
    return static_cast<u16>(std::roundf(static_cast<float>(samples) / samples_per_bit));
}
