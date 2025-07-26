// src/utils.cpp
#include "utils.hpp"
#include <zlib.h>
#include <stdexcept>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <cstring>
#include <sstream>
#include <openssl/hmac.h>
#include <openssl/evp.h>

// Helper function to get current timestamp in milliseconds
std::string get_nonce() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return std::to_string(millis);
}

// Helper function to perform HMAC SHA384 signing
std::string hmac_sha384(const std::string& key, const std::string& data) {
    unsigned char* digest;
    digest = HMAC(EVP_sha384(), key.c_str(), key.length(),
                  reinterpret_cast<const unsigned char*>(data.c_str()), data.length(), NULL, NULL);
    std::stringstream ss;
    for(int i = 0; i < 48; ++i) { // SHA384 produces 48-byte digest
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)digest[i];
    }
    return ss.str();
}

// Callback function for cURL to write received data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string compress_string(const std::string& str) {
    if (str.empty()) return "";

    z_stream zs;                        // zlib stream
    memset(&zs, 0, sizeof(zs));

    if (deflateInit(&zs, Z_BEST_COMPRESSION) != Z_OK)
        throw(std::runtime_error("deflateInit failed while compressing."));

    zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(str.data()));
    zs.avail_in = str.size();

    int ret;
    char outbuffer[32768];
    std::string outstring;

    // Retrieve the compressed bytes blockwise
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = deflate(&zs, Z_FINISH);

        if (outstring.size() < zs.total_out) {
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }
    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END) {          // an error occurred that was not EOF
        throw(std::runtime_error("Exception during zlib compression: " + std::to_string(ret)));
    }

    return outstring;
}