#pragma once

#include <pipe/Interpreter.hpp>
#include <util/Logger.hpp>
#include <util/Point.hpp>
#include <util/Size.hpp>
#include <util/Types.hpp>

namespace Pipe {

enum class ClickEvent {
    MonitorInput,
    MonitorOutput,
    Configure,
    Invalid
};

enum class Monitor {
    Input,
    Output,
    Either
};

class GenericComponent {
public:
    explicit GenericComponent(std::string name)
        : m_name(name)
        , m_log("Component <" + name + ">") {}
    virtual ~GenericComponent() = default;

    GenericComponent(const GenericComponent&) = delete;
    GenericComponent(GenericComponent&& other)
        : GenericComponent() { swap(*this, other); }

    GenericComponent& operator=(const GenericComponent&) = delete;
    GenericComponent& operator=(GenericComponent&& to_move) {
        swap(*this, to_move);
        return *this;
    }

    friend void swap(GenericComponent& first, GenericComponent& second) {
        std::swap(first.m_log, second.m_log);
        std::swap(first.m_name, second.m_name);
        std::swap(first.m_input_sample_rate, second.m_input_sample_rate);
        std::swap(first.m_output_sample_rate, second.m_output_sample_rate);
        std::swap(first.m_id, second.m_id);
    }

    static Monitor current_monitor() { return s_monitor; }
    static int current_monitor_id() { return s_monitor_id; }
    static bool monitoring(int id, Monitor monitor);
    static void set_interpreter_index(u8 index) { s_interpreter_index = index; }
    static u8 interpreter_index() { return s_interpreter_index; }
    static const InterpreterProperties& current_interpreter() { return s_interpreter; }
    static void abort_processing() { s_abort_processing = true; }
    static void prepare_processing() { s_abort_processing = false; }
    static bool did_abort_processing() { return s_abort_processing; }

    void draw(Point location);
    void monitor(Monitor monitor) { GenericComponent::set_monitor(id(), monitor, properties(monitor)); }
    Util::Point absolute_position() const { return m_absolute_position; }

    const std::string& name() const { return m_name; };
    int id() const { return m_id; };
    SampleRate output_sample_rate() const { return m_output_sample_rate; }
    SampleRate input_sample_rate() const { return m_input_sample_rate; }

    virtual bool clicked_component(Point clicked_at, ClickEvent event);
    virtual SampleRate init(SampleRate input_sample_rate, int& id_counter) = 0;
    virtual Size calculate_size() = 0;
    virtual InterpreterProperties properties(Monitor) const = 0;

protected:
    static Monitor s_monitor;
    static int s_monitor_id;
    static u8 s_interpreter_index;
    static InterpreterProperties s_interpreter;
    static inline bool s_abort_processing { false };

    static void set_monitor(int id, Monitor, InterpreterProperties);

    const Logger& logger() const { return m_log; }
    void set_input_sample_rate(SampleRate input) { m_input_sample_rate = input; }
    void set_output_sample_rate(SampleRate output) { m_output_sample_rate = output; }
    void set_id(int id) { m_id = id; }

    virtual void draw_at(Point) = 0;
    virtual void show_config_dialog();

private:
    GenericComponent()
        : m_name("INVALID")
        , m_log("ComponentBase <INVALID>") {}

    std::string m_name;
    Logger m_log;

    int m_id { -1 };
    SampleRate m_input_sample_rate { 0 };
    SampleRate m_output_sample_rate { 0 };
    Point m_absolute_position;
};

enum class MarkerType {
    NoMarker,
    MarkInput,
    MarkOutput
};

static constexpr u8 component_horizontal_spacing { 20 };
static constexpr u8 component_vertical_spacing { 10 };

void draw_connecting_line_vertical(u32 from_x, u32 from_y, u32 to_y);
void draw_connecting_line_horizontal(u32 from_x, u32 from_y, u32 to_x);
void draw_simple_connector(Point from, Point to);
void draw_marker(MarkerType type, Point pointing_at);

}
