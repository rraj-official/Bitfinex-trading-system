// include/utils.hpp
#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

// Helper function to get current timestamp in milliseconds
std::string get_nonce();

// Helper function to perform HMAC SHA384 signing
std::string hmac_sha384(const std::string& key, const std::string& data);

// Callback function for cURL to write received data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

// Compression utility
std::string compress_string(const std::string& str);

#endif // UTILS_HPP
