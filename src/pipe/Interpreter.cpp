#include "Interpreter.hpp"
#include <util/Cmplx.hpp>

using namespace Pipe;

template<>
Interpreter<float> Pipe::interpreter() {
    return { { .names = { "Float" }, .color = 0xE6993300 }, [](u8, float& f) { return f; } };
}

template<>
Interpreter<Cmplx> Pipe::interpreter() {
    return { { .names = { "Real", "Imaginary" }, .color = 0x8080CC00 }, [](u8 index, Cmplx& c) { return c[index]; } };
}

template<>
Interpreter<bool> Pipe::interpreter() {
    return { { .names = { "Bool" }, .color = 0xE64C4C00 }, [](u8, bool& b) { return b ? 1.f : 0.f; } };
}

template<>
Interpreter<u8> Pipe::interpreter() {
    return { { .names = { "Integer" }, .color = 0x66CC6600 }, [](u8, u8& b) { return static_cast<float>(b); } };
}
