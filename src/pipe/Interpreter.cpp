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
