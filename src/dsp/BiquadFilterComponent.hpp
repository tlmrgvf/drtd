#pragma once

#include <Fl/fl_draw.H>
#include <dsp/Biquad.hpp>
#include <pipe/Component.hpp>
#include <ui/BiquadFilterDialog.hpp>

namespace Dsp::Biquad {

class FilterBase {
public:
    FilterBase(Type type, float center, float parameter)
        : m_type(type)
        , m_center(center)
        , m_parameter(parameter) {}
    virtual ~FilterBase() = default;
    virtual void recalculate() = 0;

    void set_type(Type type) { m_type = type; }
    void set_center(float center) { m_center = center; }
    void set_parameter(float parameter) { m_parameter = parameter; }

    Type type() const { return m_type; }
    float center() const { return m_center; }
    float parameter() const { return m_parameter; }
    SampleRate sample_rate() const { return m_sample_rate; }

protected:
    Type m_type;
    float m_center;
    float m_parameter;
    SampleRate m_sample_rate { 0 };
};

template<typename T>
class FilterComponent final : public RefableComponent<T, T, FilterBase>
    , public FilterBase {
public:
    FilterComponent(Type type, float center, float parameter)
        : RefableComponent<T, T, FilterBase>("Biquad Filter")
        , FilterBase(type, center, parameter) {
    }

    virtual Size calculate_size() override {
        return size;
    }

    virtual void recalculate() override {
        m_filter.set_type(m_type);
        m_filter.set_center(m_center);
        m_filter.set_parameter(m_parameter);
        m_filter.set_sample_rate(this->input_sample_rate());
        m_filter.recalculate_coefficients();
        Ui::BiquadFilterDialog::update_dialog();
    }

protected:
    FilterBase& ref() override {
        return *this;
    }

    virtual SampleRate on_init(SampleRate input_sample_rate, int&) override {
        m_sample_rate = input_sample_rate;
        recalculate();
        return input_sample_rate;
    }

    virtual void draw_at(Point p) override {
        fl_rect(p.x(), p.y(), size.w(), size.h());
        p.translate(4, 4);

        auto inner_size = size;
        inner_size.resize(-8, -8);

        const unsigned one_third_width = inner_size.w() / 3;
        const unsigned one_quarter_width = inner_size.w() / 4;
        const unsigned three_quarters_width = 3 * one_quarter_width;
        const unsigned two_thirds_width = 2 * one_third_width;
        const unsigned half_width = inner_size.w() / 2;

        fl_push_matrix();
        fl_translate(p.x(), p.y());
        fl_begin_line();

        switch (m_filter.type()) {
        case Type::Lowpass:
            fl_vertex(0, 0);
            fl_vertex(two_thirds_width, 0);
            fl_vertex(three_quarters_width, inner_size.h());
            break;
        case Type::Highpass:
            fl_vertex(one_quarter_width, inner_size.h());
            fl_vertex(one_third_width, 0);
            fl_vertex(inner_size.w(), 0);
            break;
        case Type::BandpassPeak:
            fl_line(p.x(), p.y(), p.x() + inner_size.w(), p.y());
            fl_vertex(one_third_width, inner_size.h());
            fl_vertex(half_width, 0);
            fl_vertex(two_thirds_width, inner_size.h());
            break;
        case Type::BandpassSkirt:
            fl_line(p.x(), p.y() + 10, p.x() + inner_size.w(), p.y() + 10);
            fl_vertex(one_third_width, inner_size.h());
            fl_vertex(half_width, 0);
            fl_vertex(two_thirds_width, inner_size.h());
            break;
        case Type::Notch:
            fl_vertex(0, 0);
            fl_vertex(one_third_width, 0);
            fl_vertex(half_width, inner_size.h());
            fl_vertex(two_thirds_width, 0);
            fl_vertex(inner_size.w(), 0);
            break;
        default:
            assert(false);
        }

        fl_end_line();
        fl_pop_matrix();
    }

    virtual void show_config_dialog() override {
        Ui::BiquadFilterDialog::show_dialog(this->make_ref());
    }

    virtual T process(T sample) override {
        return m_filter.filter_sample(sample);
    }

private:
    static constexpr Size size { 40, 25 };

    Filter<T> m_filter;
};

}
