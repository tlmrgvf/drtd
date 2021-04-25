#pragma once

#include <pipe/Component.hpp>
#include <util/Cmplx.hpp>

namespace Dsp {

class AngleDifference final : public ComponentBase<Cmplx, float> {
public:
    AngleDifference();
    virtual Size calculate_size() override;

protected:
    virtual void draw_at(Point) override;
    virtual float process(Cmplx) override;

private:
    static constexpr Size size { 30, 20 };

    float m_previous_angle { 0 };
};

}
