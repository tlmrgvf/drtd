#pragma once

#include <Fl/Fl.H>
#include <functional>
#include <string>
#include <util/Buffer.hpp>
#include <util/Types.hpp>

namespace Pipe {

struct InterpreterProperties {
    Buffer<std::string> names;
    Fl_Color color;
};

template<typename Type>
struct Interpreter : InterpreterProperties {
    std::function<float(u8, Type&)> interpreter_function;
};

template<typename Type>
Interpreter<Type> interpreter();

}
