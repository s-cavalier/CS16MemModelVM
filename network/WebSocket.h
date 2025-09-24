#ifndef __WEBSOCKET_H__
#define __WEBSOCKET_H__
#include <string>
#include <vector>
#include <stdexcept>
#include <array>

struct BadWebSocketRequest : std::exception {
    const char* field;

    BadWebSocketRequest(const char* field) : field(field) {}
    const char* what() const noexcept override { return field; }
};

// Parses websocket and checks for errors
// Will throw BadWebSocketRequest on failure
std::string parseWebSocketRequest(const std::string& req, const char* expectedRoute);

std::string buildWebSocketResponse(const std::string& key);

std::vector<char> convertToPacket(const std::string& str);

struct ClientToServerPacket {
    enum Opcode : char {
        CONTINUE = 0,
        TEXT_FRAME = 1,
        BINARY_FRAME = 2,
        CONN_CLOSE = 8,
        PING = 9,
        PONG = 10,
    };
    
    std::vector<char> payload;
    uint8_t maskKey[4];
    Opcode opcode{};
    bool fin{};
    bool mask{};
    bool failedParse{};

    // Unpack a websocket packet byte array into a struct
    ClientToServerPacket(const std::vector<char>& bytes);
};

inline static const std::vector<char> disconnectHeader { char(0x88), 0x02, 0x03, char(0xE8) };

template <std::size_t N>
constexpr auto convertToPacket(const char (&str)[N]) {
    constexpr std::size_t payload_len = N - 1; // drop null terminator

    if constexpr (payload_len <= 125) {
        std::array<uint8_t, 2 + payload_len> frame = {};
        frame[0] = 0x81;  // FIN + text
        frame[1] = static_cast<uint8_t>(payload_len);  // no mask bit
        for (std::size_t i = 0; i < payload_len; i++) {
            frame[2 + i] = static_cast<uint8_t>(str[i]);
        }
        return frame;
    } 
    else if constexpr (payload_len <= 0xFFFF) {
        std::array<uint8_t, 4 + payload_len> frame = {};
        frame[0] = 0x81;  
        frame[1] = 126;   // 126 marker for 16-bit length
        frame[2] = static_cast<uint8_t>((payload_len >> 8) & 0xFF);
        frame[3] = static_cast<uint8_t>(payload_len & 0xFF);
        for (std::size_t i = 0; i < payload_len; i++) {
            frame[4 + i] = static_cast<uint8_t>(str[i]);
        }
        return frame;
    } 
    else {
        std::array<uint8_t, 10 + payload_len> frame = {};
        frame[0] = 0x81;
        frame[1] = 127;   // 127 marker for 64-bit length
        // network byte order (big endian) for 8 bytes
        for (int i = 0; i < 8; i++) {
            frame[2 + i] = static_cast<uint8_t>((payload_len >> (56 - i * 8)) & 0xFF);
        }
        for (std::size_t i = 0; i < payload_len; i++) {
            frame[10 + i] = static_cast<uint8_t>(str[i]);
        }
        return frame;
    }
}

 

#endif