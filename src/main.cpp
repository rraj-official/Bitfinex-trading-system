// src/main.cpp
#include <iostream>
#include <string>
#include <limits>
#include <chrono>
#include "api.hpp"
#include "websocket_server.hpp"
#include "thread_safe_queue.hpp"
#include <curl/curl.h>


// Define a structure to hold order book data
struct OrderBookData {
    std::string symbol;
    std::string data;
};

// Create a thread-safe queue for order book data
ThreadSafeQueue<OrderBookData> orderbook_queue;

// Atomic flag to control the update loop
std::atomic<bool> stop_updates;

// Function to fetch and enqueue order book data asynchronously
void fetch_orderbook_async(const std::string& symbol) {
    // Launch an asynchronous task to fetch the order book
    std::async(std::launch::async, [symbol]() {
        std::ostringstream oss;
        oss << "Fetching orderbook for symbol: " << symbol;
        std::string orderbook_data = oss.str();
        
        // Replace with actual API call
        get_orderbook(symbol);  // Ensure this function is optimized for speed

        // Enqueue the fetched data
        orderbook_queue.enqueue(OrderBookData{symbol, orderbook_data});
    });
}



// Function to process and send order book updates
void process_orderbook_updates(WebSocketServer& ws_server) {
    OrderBookData ob_data;
    while (orderbook_queue.dequeue(ob_data)) {
        ws_server.send_orderbook_update(ob_data.symbol, ob_data.data);
    }
}

// Modified orderbook_update_task to increase update frequency and use async fetching
void orderbook_update_task(WebSocketServer& ws_server, const std::string& symbol) {
    while (!stop_updates) {
        // Fetch the order book from the Bitfinex API (use actual API call here)
        std::ostringstream oss;
        oss << "Fetching orderbook for symbol: " << symbol;
        std::string orderbook_data = oss.str();

        // Simulate fetching orderbook data
        get_orderbook(symbol);  // Implement orderbook fetching logic

        // Send the orderbook data to subscribed clients
        ws_server.send_orderbook_update(symbol, orderbook_data);

        // Sleep for 100 milliseconds before fetching the next update for faster updates
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "Orderbook updates stopped for " << symbol << std::endl;
    std::cout << std::endl;
}

// Function to wait for Enter key press to stop updates
void wait_for_enter() {
    std::cin.get();  // Wait for the Enter key press
    stop_updates = true;  // Set the atomic flag to stop updates
}

// Function to display the menu
void display_menu() {
    // Menu Header with an Intro
    std::cout << "==============================================\n";
    std::cout << "||  Welcome to the Bitfinex Trading System  ||\n";
    std::cout << "==============================================\n";
    std::cout << "Created by: Rohit Raj\n";
    std::cout << "Purpose: For paper trading on the Bitfinex platform.\n";
    std::cout << "==============================================\n";
    
    // Options Menu
    std::cout << "\nChoose an option below to get started:\n";
    std::cout << "-----------------------------------------\n";
    std::cout << "1. 📈  Place Buy Order\n";
    std::cout << "2. 📉  Place Sell Order\n";
    std::cout << "3. 🛠️  Modify Order\n";
    std::cout << "4. ❌  Cancel Order\n";
    std::cout << "5. 📊  Get Order Book\n";
    std::cout << "6. 📑  View Current Positions\n";
    std::cout << "7. 🧬  Connect WebSocket Client and Subscribe to Symbol\n";
    std::cout << "0. 🚪  Exit\n";
    std::cout << "-----------------------------------------\n";
    std::cout << "Enter your choice: ";
}

int main() {
    // Initialize cURL
    curl_global_init(CURL_GLOBAL_DEFAULT);

    WebSocketServer ws_server;

    // Start WebSocket server in a separate thread
    std::thread ws_thread([&ws_server]() {
        ws_server.run(9002);  // Run WebSocket server on port 9002
    });

    int choice;
    long int order_id;
    bool running = true;

    while (running) {
        display_menu();
        std::cin >> choice;
        std::cout << std::endl;
        // Clear input buffer to handle any extraneous input
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
            case 1: { // Place Buy Order
                std::string symbol, type;
                double amount, price;
                int trade_type;
                // Display trading type menu
                std::cout << "--- Select Trading Type (Buy)---\n";
                std::cout << "1. Spot Trading\n";
                std::cout << "2. Futures/Options Trading\n";
                std::cout << "Enter your choice (1 or 2): ";
                std::cin >> trade_type;

                // Based on the choice, provide appropriate symbol format
                if (trade_type == 1) {
                    // Spot Trading
                    std::cout << "Enter Symbol (supported: tTESTBTC:TESTUSD): ";
                    std::cin >> symbol;
                    std::cout << "Enter order type (e.g., EXCHANGE LIMIT): ";
                    std::cin.ignore(); // Ignore leftover newline from previous input
                    std::getline(std::cin, type);
                    std::cout << "Enter amount: ";
                    std::cin >> amount;
                    std::cout << "Enter price: ";
                    std::cin >> price;
                    order_id = place_order(symbol, type, amount, price);
                    if (order_id != -1) {
                        std::cout << "\n--- Buy Order Summary ---\n";
                        std::cout << "Symbol       : " << symbol << std::endl;
                        std::cout << "Order Type   : " << type << std::endl;
                        std::cout << "Amount       : " << amount << std::endl;
                        std::cout << "Price        : " << price << std::endl;
                        std::cout << "Order Status : Buy order placed successfully!\n";
                        std::cout << "Order ID     : " << order_id << std::endl;
                        std::cout << std::endl;
                    } else {
                        std::cout << "Error placing buy order. Please check your input and try again.\n";
                    }
                    // Prompt user to press Enter to continue
                    std::cout << "\nPress Enter to continue...";
                    std::cin.ignore(); // Wait for user to press Enter
                    std::cin.get();    // Wait for additional input before proceeding
                    break;
                } else if (trade_type == 2) {
                    // Futures/Options Trading
                    std::cout << "Enter Symbol (supported: tTESTBTCF0:TESTUSDTF0): ";
                    std::cin >> symbol;
                    std::cout << "Enter order type (e.g., EXCHANGE LIMIT): ";
                    std::cin.ignore(); // Ignore leftover newline from previous input
                    std::getline(std::cin, type);
                    std::cout << "Enter amount: ";
                    std::cin >> amount;
                    std::cout << "Enter price: ";
                    std::cin >> price;
                    order_id = place_order(symbol, type, amount, price);
                    if (order_id != -1) {
                        std::cout << "\n--- Buy Order Summary ---\n";
                        std::cout << "Symbol       : " << symbol << std::endl;
                        std::cout << "Order Type   : " << type << std::endl;
                        std::cout << "Amount       : " << amount << std::endl;
                        std::cout << "Price        : " << price << std::endl;
                        std::cout << "Order Status : Buy order placed successfully!\n";
                        std::cout << "Order ID     : " << order_id << std::endl;
                        std::cout << std::endl;
                    } else {
                        std::cout << "Error placing buy order. Please check your input and try again.\n";
                    }
                    // Prompt user to press Enter to continue
                    std::cout << "\nPress Enter to continue...";
                    std::cin.ignore(); // Wait for user to press Enter
                    std::cin.get();    // Wait for additional input before proceeding
                    break;
                } else {
                    // Invalid choice, handle error
                    std::cout << "Invalid choice. Please select either 1 for Spot or 2 for Futures/Options.\n";
                    break;
                }
            }
            case 2: { // Place Sell Order
                std::string symbol, type;
                double amount, price;
                int trade_type;
                // Display trading type menu
                std::cout << "--- Select Trading Type (Sell)---\n";
                std::cout << "1. Spot Trading\n";
                std::cout << "2. Futures/Options Trading\n";
                std::cout << "Enter your choice (1 or 2): ";
                std::cin >> trade_type;

                // Based on the choice, provide appropriate symbol format
                if (trade_type == 1) {
                    // Spot Trading
                    std::cout << "Enter Symbol (supported: tTESTBTC:TESTUSD): ";
                    std::cin >> symbol;
                    std::cout << "Enter order type (e.g., EXCHANGE LIMIT): ";
                    std::cin.ignore(); // Ignore leftover newline from previous input
                    std::getline(std::cin, type);
                    std::cout << "Enter amount: ";
                    std::cin >> amount;
                    std::cout << "Enter price: ";
                    std::cin >> price;
                    order_id = place_order(symbol, type, amount, price);
                    if (order_id != -1) {
                        std::cout << "\n--- Sell Order Summary ---\n";
                        std::cout << "Symbol       : " << symbol << std::endl;
                        std::cout << "Order Type   : " << type << std::endl;
                        std::cout << "Amount       : " << amount << std::endl;
                        std::cout << "Price        : " << price << std::endl;
                        std::cout << "Order Status : Sell order placed successfully!\n";
                        std::cout << "Order ID     : " << order_id << std::endl;
                        std::cout << std::endl;
                    } else {
                        std::cout << "Error placing sell order. Please check your input and try again.\n";
                    }
                    // Prompt user to press Enter to continue
                    std::cout << "\nPress Enter to continue...";
                    std::cin.ignore(); // Wait for user to press Enter
                    std::cin.get();    // Wait for additional input before proceeding
                    break;
                } else if (trade_type == 2) {
                    // Futures/Options Trading
                    std::cout << "Enter Symbol (supported: tTESTBTCF0:TESTUSDTF0): ";
                    std::cin >> symbol;
                    std::cout << "Enter order type (e.g., EXCHANGE LIMIT): ";
                    std::cin.ignore(); // Ignore leftover newline from previous input
                    std::getline(std::cin, type);
                    std::cout << "Enter amount: ";
                    std::cin >> amount;
                    std::cout << "Enter price: ";
                    std::cin >> price;
                    order_id = place_order(symbol, type, amount, price);
                    if (order_id != -1) {
                        std::cout << "\n--- Sell Order Summary ---\n";
                        std::cout << "Symbol       : " << symbol << std::endl;
                        std::cout << "Order Type   : " << type << std::endl;
                        std::cout << "Amount       : " << amount << std::endl;
                        std::cout << "Price        : " << price << std::endl;
                        std::cout << "Order Status : Sell order placed successfully!\n";
                        std::cout << "Order ID     : " << order_id << std::endl;
                        std::cout << std::endl;
                    } else {
                        std::cout << "Error placing sell order. Please check your input and try again.\n";
                    }
                    // Prompt user to press Enter to continue
                    std::cout << "\nPress Enter to continue...";
                    std::cin.ignore(); // Wait for user to press Enter
                    std::cin.get();    // Wait for additional input before proceeding
                    break;
                } else {
                    // Invalid choice, handle error
                    std::cout << "Invalid choice. Please select either 1 for Spot or 2 for Futures/Options.\n";
                    break;
                }
            }
            case 3: { // Modify Order
                double new_amount, new_price;

                std::cout << "Enter order ID to modify: ";
                std::cin >> order_id;
                std::cout << "Enter new amount: ";
                std::cin >> new_amount;
                std::cout << "Enter new price: ";
                std::cin >> new_price;

                modify_order(order_id, new_amount, new_price);
                // Prompt user to press Enter to continue
                std::cout << "\nPress Enter to continue...";
                std::cin.ignore(); // Clear the buffer
                std::cin.get();    // Wait for user input
                break;
            }
            case 4: { // Cancel Order

                std::cout << "Enter order ID to cancel: ";
                std::cin >> order_id;

                cancel_order(order_id);
                // Prompt user to press Enter to continue
                std::cout << "\nPress Enter to continue...";
                std::cin.ignore(); // Clear the buffer
                std::cin.get();    // Wait for user input
                break;
            }
            case 5: { // Get Order Book
                std::string symbol;
                int limit;

                std::cout << "Enter symbol: ";
                std::cin >> symbol;
                std::cout << "Enter limit (default 25): ";
                std::cin >> limit;

                get_orderbook(symbol, limit);
                // Prompt user to press Enter to continue
                std::cout << "\nPress Enter to continue...";
                std::cin.ignore(); // Clear the buffer
                std::cin.get();    // Wait for user input
                break;
            }
            case 6: { // View Current Positions
                view_positions();
                // Prompt user to press Enter to continue
                std::cout << "\nPress Enter to continue...";
                std::cin.ignore(); // Clear the buffer
                std::cin.get();    // Wait for user input
                break;
            }
            case 7: {  // Connect WebSocket Client and Subscribe to Symbol
                std::string symbol;
                std::cout << "Enter symbol to subscribe to (e.g., tBTCUSD): ";
                std::cin >> symbol;
                std::cin.ignore();  // Ignore extra newline

                // Reset the stop_updates flag and start the orderbook update task
                stop_updates = false;

                // Start the orderbook update task in a separate thread
                std::thread update_thread(orderbook_update_task, std::ref(ws_server), symbol);

                // Wait for Enter key to stop updates
                std::thread enter_thread(wait_for_enter);

                // Join both threads
                enter_thread.join();
                update_thread.join();

                std::cout << "Exited WebSocket client for symbol: " << symbol << std::endl;
                break;
            }
            case 0: // Exit
                std::cout << "Exiting application...\n";
                // Set stop flag for updates
                stop_updates = true;

                // Stop the WebSocket server
                ws_server.stop();

                running = false;
                break;
            default:
                std::cout << "Invalid choice. Please try again." << std::endl;
                // Prompt user to press Enter to continue
                std::cout << "\nPress Enter to continue...";
                std::cin.ignore(); // Clear the buffer
                std::cin.get();    // Wait for user input
                break;
        }
    }
    if (ws_thread.joinable()) {
        ws_thread.join();
    }
    // Cleanup cURL
    curl_global_cleanup();
    return 0;
}
