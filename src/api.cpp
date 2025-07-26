// src/api.cpp
#include "api.hpp"
#include "utils.hpp"
#include "json.hpp" 
#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>

using json = nlohmann::json;

// Replace with your actual API credentials
const std::string API_KEY = "491b8c278cd9bd7562e34703c686f9bcd6799577ee3";
const std::string API_SECRET = "84817cc462ee80806bfbf7154817ea36e7531b6c531";
const std::string API_URL = "https://api.bitfinex.com/v2";

// Function to perform HTTP POST requests
json http_post(const std::string& endpoint, const json& payload, bool auth) {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if(curl) {
        std::string url = API_URL + "/" + endpoint;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        // For HTTPS
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        // HTTP POST
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        std::string postFields = payload.dump();
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        if(auth) {
            std::string nonce = get_nonce();
            std::string body = "/api/v2/" + endpoint + nonce + payload.dump();
            std::string signature = hmac_sha384(API_SECRET, body);
            headers = curl_slist_append(headers, ("bfx-apikey: " + API_KEY).c_str());
            headers = curl_slist_append(headers, ("bfx-nonce: " + nonce).c_str());
            headers = curl_slist_append(headers, ("bfx-signature: " + signature).c_str());
        }

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Callback to capture response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Perform the request
        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            std::cerr << "cURL Error: " << curl_easy_strerror(res) << std::endl;

        // Cleanup
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }

    // Parse the response
    try {
        return json::parse(readBuffer);
    } catch(json::parse_error& e) {
        std::cerr << "JSON Parse Error: " << e.what() << std::endl;
        return nullptr;
    }
}

// Function to place a limit order
long int place_order(const std::string& symbol, const std::string& type, double amount, double price) {
    json payload = {
        {"type", type}, // e.g., "EXCHANGE LIMIT"
        {"symbol", symbol},
        {"amount", std::to_string(amount)},
        {"price", std::to_string(price)}
        // You can add more optional fields here
    };
    std::cout << "Place Order Request: " << payload.dump() << std::endl;
    json response = http_post("auth/w/order/submit", payload, true);
    std::cout << "Place Order Response: " << response.dump(4) << std::endl;

    // Extract order_id from response
    if (response.is_array() && response.size() > 0 && response[0].is_number()) {
        return response[4][0][0].get<long int>();
    } else {
        std::cerr << "Failed to retrieve order_id from response." << std::endl;
        return -1;
    }
}

// Function to modify an existing order
void modify_order(long int order_id, double new_amount, double new_price) {
    json payload = {
        {"id", order_id},
        {"amount", std::to_string(new_amount)},
        {"price", std::to_string(new_price)}
        // You can add more fields as needed
    };

    std::cout << "Modify Order Request: " << payload.dump() << std::endl;
    json response = http_post("auth/w/order/update", payload, true);
    std::cout << "Modify Order Response: " << response.dump(4) << std::endl;

    // Check if the response indicates success
    if (response.is_array() && response.size() >= 7 && response[6] == "SUCCESS") {
        std::cout << "\n--- Modify Order Summary ---\n";
        std::cout << "New Amount       : " << new_amount << std::endl;
        std::cout << "New Price        : " << new_price << std::endl;
        std::cout << "Order Status     : Order modified successfully!\n";
        std::cout << "Order ID         : " << order_id << std::endl;
        std::cout << std::endl;
    } else {
        // Handle unsuccessful modification
        std::cerr << "\nFailed to modify order. Please check the order details and try again.\n";
        if (response.size() > 6 && response[6].is_string()) {
            std::cerr << "Error Message: " << response[6].get<std::string>() << std::endl;
        }
        std::cerr << std::endl;
    }
}

// Function to cancel an order
void cancel_order(long int order_id) {
    json payload = {
        {"id", order_id}
    };

    std::cout << "Cancel Order Request: " << payload.dump() << std::endl;
    json response = http_post("auth/w/order/cancel", payload, true);
    std::cout << "Cancel Order Response: " << response.dump(4) << std::endl;
    
    // Check if the response indicates success
    if (response.is_array() && response.size() >= 7 && response[6] == "SUCCESS") {
        std::cout << "\n--- Cancel Order Summary ---\n";
        std::cout << "Order Status : Order cancelled successfully!\n";
        std::cout << "Order ID     : " << order_id << std::endl;
        std::cout << std::endl;
    } else {
        // Handle unsuccessful cancellation
        std::cerr << "\nFailed to cancel order. Please check the order ID and try again.\n";
        if (response.size() > 6 && response[6].is_string()) {
            std::cerr << "Error Message: " << response[6].get<std::string>() << std::endl;
        }
        std::cerr << std::endl;
    }
}

// Function to get the order book for a symbol
void get_orderbook(const std::string& symbol, int limit) {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if(curl) {
        std::ostringstream url_stream;
        url_stream << API_URL << "/book/" << symbol << "/P0?len=" << limit;
        std::string url = url_stream.str();
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        // For HTTPS
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        // HTTP GET
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

        // Callback to capture response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // Perform the request
        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            std::cerr << "cURL Error: " << curl_easy_strerror(res) << std::endl;

        // Cleanup
        curl_easy_cleanup(curl);
    }

    // Parse and display the order book
    try {
        json orderbook = json::parse(readBuffer);
        std::cout << "Order Book for " << symbol << ":\n" << orderbook.dump(4) << std::endl;
    } catch(json::parse_error& e) {
        std::cerr << "JSON Parse Error: " << e.what() << std::endl;
    }
}

// Function to view current positions
void view_positions() {
    json payload = {}; // No payload needed for positions
    json response = http_post("auth/r/positions", payload, false);

    if (response.is_array()) {
        if (response.size() >= 3) {
            std::cout << "\nNo current open positions\n" <<std::endl;
        } else {
            std::cout << "Current Positions: " << response.dump(4) << std::endl;
        }
    } else {
        std::cerr << "Unexpected response format." << std::endl;
    }
}
