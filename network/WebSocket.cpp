#include "WebSocket.h"
#include <sstream>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <netdb.h>
#include <limits>
#include <unordered_map>

std::string parseWebSocketRequest(const std::string& req, const char* expectedRoute) {
    // Split into lines
    std::istringstream iss(req);

    std::string val;
    bool err = std::getline(iss, val).eof();

    if (err) throw BadWebSocketRequest("Empty request");

    if ( val.find("GET") == std::string::npos) throw BadWebSocketRequest("Bad or no method");
    if ( val.find(expectedRoute, 3) == std::string::npos ) throw BadWebSocketRequest("Bad or no route");
    if ( val.find("HTTP/1.1", 3) == std::string::npos ) throw BadWebSocketRequest("Bad or no HTTP version");

    std::unordered_map<std::string, std::string> headers;

    while (std::getline(iss, val)) {
        val.pop_back();
        size_t loc = val.find(' ');
        if (loc == std::string::npos) break;
        std::string left = val.substr(0, loc);
        std::string right = val.substr( loc + 1 );

        headers[left] = right;
    }

    if ( headers.count("Sec-WebSocket-Version:") == 0 || headers["Sec-WebSocket-Version:"] != "13") throw BadWebSocketRequest("Bad or no websocket version");
    if ( headers.count("Sec-WebSocket-Key:") == 0) throw BadWebSocketRequest("No websocket key");
    if ( headers.count("Connection:") == 0 || headers["Connection:"] != "Upgrade") throw BadWebSocketRequest("Bad or no connection header");
    if ( headers.count("Upgrade:") == 0 || headers["websocket:"] == "13") throw BadWebSocketRequest("Bad or no websocket upgrade header");

    return headers["Sec-WebSocket-Key:"];
}

std::string buildWebSocketResponse(const std::string& key) {
    std::string res = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    unsigned char sha1_res[SHA_DIGEST_LENGTH];
    SHA1((const unsigned char*)res.data(), res.size(), sha1_res);

    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* mem = BIO_new(BIO_s_mem());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    BIO_push(b64, mem);

    BIO_write(b64, sha1_res, SHA_DIGEST_LENGTH);
    if (BIO_flush(b64) != 1) {
        BIO_free_all(b64);
        throw std::runtime_error("BIO_flush failed");
    }

    BUF_MEM* buffer_ptr;
    BIO_get_mem_ptr(mem, &buffer_ptr);

    std::string accept_key(buffer_ptr->data, buffer_ptr->length);

    BIO_free_all(b64); 

    std::string response =
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: " + accept_key + "\r\n\r\n";

    return response;
}


ClientToServerPacket::ClientToServerPacket(const std::vector<char>& bytes) {
    if (bytes.size() < 2) { failedParse = true; return; }

    fin = (bytes[0] & 0x80) != 0;
    opcode = Opcode(bytes[0] & 0x0F);
    mask = (bytes[1] & 0x80) != 0;

    uint64_t payloadLen = 0;
    size_t offset = 2;

    uint8_t tpayload = bytes[1] & 0x7F;
    if (tpayload <= 125) {
        payloadLen = tpayload;
    } else if (tpayload == 126) {
        if (bytes.size() < 4) { failedParse = true; return; }
        payloadLen = (uint16_t(uint8_t(bytes[2])) << 8) | uint8_t(bytes[3]);
        offset = 4;
    } else {
        if (bytes.size() < 10) { failedParse = true; return; }
        payloadLen =
            (uint64_t(uint8_t(bytes[2])) << 56) |
            (uint64_t(uint8_t(bytes[3])) << 48) |
            (uint64_t(uint8_t(bytes[4])) << 40) |
            (uint64_t(uint8_t(bytes[5])) << 32) |
            (uint64_t(uint8_t(bytes[6])) << 24) |
            (uint64_t(uint8_t(bytes[7])) << 16) |
            (uint64_t(uint8_t(bytes[8])) << 8)  |
            uint64_t(uint8_t(bytes[9]));
        offset = 10;
    }

    uint8_t maskKey[4] = {0};
    if (mask) {
        if (bytes.size() < offset + 4) { failedParse = true; return; }
        for (int i = 0; i < 4; i++) {
            maskKey[i] = uint8_t(bytes[offset + i]);
        }
        offset += 4;
    }

    if (bytes.size() < offset + payloadLen) { failedParse = true; return; }

    payload.resize(payloadLen);
    for (size_t i = 0; i < payloadLen; i++) {
        payload[i] = mask ? (bytes[offset + i] ^ maskKey[i % 4]) : bytes[offset + i];
    }
}

std::vector<char> convertToPacket(const std::string& str) {
    std::vector<char> res;
    res.reserve(14);
    res.resize(2);
      
    res[0] = ( 1 << 7 ) | ClientToServerPacket::TEXT_FRAME;

    if (str.size() <= 125) res[1] |= (unsigned char)(str.size());
    else if (str.size() <= UINT16_MAX) {
        res.push_back( str.size() >> 8 );
        res.push_back( str.size() & 0xFF );
        res[1] |= 126;
    } else {
        res.push_back( str.size() >> 56 );
        res.push_back( (str.size() >> 48) & 0xff );
        res.push_back( (str.size() >> 40) & 0xff );
        res.push_back( (str.size() >> 32) & 0xff );
        res.push_back( (str.size() >> 24) & 0xff );
        res.push_back( (str.size() >> 16) & 0xff );
        res.push_back( (str.size() >> 8) & 0xff );
        res.push_back( str.size() & 0xff );
        res[1] |= 127;
    }

    res.insert(res.end(), str.begin(), str.end());

    return res;
}