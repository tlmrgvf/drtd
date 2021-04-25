#pragma once

#include <pipe/Component.hpp>
#include <util/Cmplx.hpp>
#include <util/Types.hpp>

namespace Dsp {

class IQMixer final : public RefableComponent<float, Cmplx, IQMixer> {
public:
    IQMixer(Hertz frequency);
    virtual Size calculate_size() override;
    Hertz frequency() const;
    void set_frequency(Hertz);

protected:
    virtual IQMixer& ref() override;
    virtual void draw_at(Point) override;
    virtual Cmplx process(float) override;
    virtual SampleRate on_init(SampleRate, int&) override;
    virtual void show_config_dialog() override;

private:
    static constexpr u8 icon_radius = 11;
    static constexpr Size size { icon_radius * 2 + 1, icon_radius * 2 };

    float m_phase { 0 };
    float m_phase_step { 0 };
    Hertz m_frequency { 0 };
};

}
