#pragma once

#include <pipe/Component.hpp>

namespace Dsp {

class NRZIDecoder final : public ComponentBase<bool, bool> {
public:
    NRZIDecoder(bool inverted);
    virtual Size calculate_size() override;

protected:
    virtual void draw_at(Point) override;
    virtual bool process(bool) override;

private:
    bool m_inverted;
    bool m_last_sample { false };
};

}
