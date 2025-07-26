// include/api.hpp
#ifndef API_HPP
#define API_HPP

#include <string>
#include "json.hpp"

using json = nlohmann::json;

// Function to perform HTTP POST requests
json http_post(const std::string& endpoint, const json& payload, bool auth=true);

// Function to place a limit order
long int place_order(const std::string& symbol, const std::string& type, double amount, double price);

// Function to modify an existing order
void modify_order(long int order_id, double new_amount, double new_price);

// Function to cancel an order
void cancel_order(long int order_id);

// Function to get the order book for a symbol
void get_orderbook(const std::string& symbol, int limit=25);

// Function to view current positions
void view_positions();

#endif // API_HPP
