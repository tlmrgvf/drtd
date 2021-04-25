#pragma once

#include "PocsagData.hpp"
#include <optional>
#include <sstream>
#include <util/Types.hpp>

namespace Dsp::PocsagProtocol {

class Message final {
public:
    friend class MessageBuilder;
    static constexpr u8 codewords_per_batch { 16 };

    enum class ContentType : u8 {
        NoContent,
        Numeric,
        AlphaNumeric,
        Both,
        __Count
    };

    std::string str() const;
    static std::string content_name(ContentType);

private:
    Message(ContentType, std::optional<Data> address, std::string alpha, std::string numeric, bool has_invalid_code_word, bool has_data, u16 baud_rate);

    ContentType m_type;
    std::optional<Data> m_address;
    std::string m_alpha;
    std::string m_numeric;
    bool m_has_invalid_code_word;
    bool m_has_data;
    u16 m_baud_rate;
};

class MessageBuilder final {
public:
    void set_has_invalid_codeword() { m_has_invalid_code_word = true; }
    void append_data(const Data&);

    bool valid() const { return m_valid; }
    Message build(Message::ContentType, BaudRate);

private:
    std::ostringstream m_alpha_builder;
    std::ostringstream m_numerical_builder;
    std::optional<Data> m_address;
    bool m_has_invalid_code_word { false };
    char m_raw_data_bits { 0 };
    u8 m_raw_bit_count { 0 };
    bool m_valid { false };
    bool m_has_data { false };
};

}
