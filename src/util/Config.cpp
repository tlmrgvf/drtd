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
#include "Buffer.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include "Types.hpp"
#include "Util.hpp"
#include <filesystem>
#include <fstream>
#include <map>

using namespace Util;

static constexpr const char* s_config_file_name { ".drtd" };
static constexpr const char s_config_magic[] { 'D', 'R', 'T', 'D' };
static const Logger s_log("Config");
static std::filesystem::path s_config_path;
static std::map<std::string, Buffer<char>> s_buffers;

Buffer<char>& Config::buffer(std::string k) {
    return s_buffers.find(k)->second;
}

bool Config::buffers_contains(std::string k) {
    return s_buffers.count(k) != 0;
}

void Config::make_buffer(std::string k, u16 size) {
    if (!buffers_contains(k))
        s_buffers.emplace(k, Buffer<char>(size));
}

void Config::setup(const char* exec_path) {
    std::filesystem::path path(exec_path);
    s_config_path = path.replace_filename(s_config_file_name);
    s_log.info() << "Executable path \""
                 << exec_path
                 << "\", parent path \""
                 << path.parent_path().c_str()
                 << "\", config path \""
                 << s_config_path.c_str()
                 << '"';
}

void Config::load_file() {
    s_log.info() << "Loading config...";
    std::ifstream conf_stream(s_config_path, std::ios::binary);
    if (!conf_stream.good()) {
        s_log.info() << "No config file found!";
        return;
    }

    auto magic_size = sizeof(s_config_magic);
    char check_magic[magic_size];
    conf_stream.read(check_magic, magic_size);
    for (size_t i = 0; i < magic_size; ++i) {
        if (check_magic[i] != s_config_magic[i])
            Util::die("Invalid config magic!");
    }

    auto start = conf_stream.tellg();
    conf_stream.seekg(0, std::ios::end);
    auto file_size = conf_stream.tellg() - start;
    conf_stream.seekg(start);

    if (!conf_stream.eof()) {
        Util::Buffer<char> file_buffer(file_size);
        conf_stream.read(file_buffer.ptr(), file_size);
        std::stringstream stringstream;

        for (long i = 0; i < file_size; ++i) {
            for (; i < file_size && file_buffer[i]; ++i)
                stringstream << file_buffer[i];

            ++i;
            if ((i + 1) >= file_size)
                break;

            u16 setting_size = static_cast<u16>((file_buffer[i + 1] << 8) | (0xFF & file_buffer[i]));
            auto str = stringstream.str();

            stringstream.str("");
            stringstream.clear();

            i += 2;
            if ((i + setting_size) > file_size)
                break;

            s_log.info() << "Loading setting \"" << str << "\" with size " << setting_size;

            Buffer<char> buffer(setting_size);
            std::memcpy(buffer.ptr(), file_buffer.ptr() + i, setting_size);
            s_buffers.emplace(str, std::move(buffer));
            i += setting_size - 1;
        }
    }
    conf_stream.close();
}

void Config::save_all() {
    s_log.info() << "Saving config...";
    std::ofstream conf_stream(s_config_path, std::ios::binary);
    for (size_t i = 0; i < sizeof(s_config_magic); ++i)
        conf_stream << s_config_magic[i];

    for (auto& s : s_buffers) {
        auto& buffer = s.second;

        u16 size = static_cast<u16>(buffer.size());
        s_log.info() << "Saving setting \"" << s.first << "\" with size " << size;
        conf_stream << s.first << '\0';
        conf_stream.write(reinterpret_cast<char*>(&size), sizeof(size));
        conf_stream.write(buffer.ptr(), size);
    }

    conf_stream.flush();
    conf_stream.close();
}
