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
    float get_current_baud_rate() const;
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
