// src/websocket_server.cpp
#include "websocket_server.hpp"
#include "api.hpp"
#include "utils.hpp"
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <iostream>
#include <mutex>

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

WebSocketServer::WebSocketServer() {
    // Initialize Asio
    m_server.init_asio();

    // Set open, close, and message handlers
    m_server.set_open_handler(bind(&WebSocketServer::on_open, this, ::_1));
    m_server.set_close_handler(bind(&WebSocketServer::on_close, this, ::_1));
    m_server.set_message_handler(bind(&WebSocketServer::on_message, this, ::_1, ::_2));
}

void WebSocketServer::run(uint16_t port) {
    m_server.listen(port);
    m_server.start_accept();
    m_server.run();
}

void WebSocketServer::on_open(websocketpp::connection_hdl hdl) {
    std::cout << "Client connected!" << std::endl;
}

void WebSocketServer::on_close(websocketpp::connection_hdl hdl) {
    std::lock_guard<std::mutex> lock(m_mutex);
    // Remove the connection from all subscription sets
    for (auto& pair : m_subscriptions) {
        pair.second.erase(hdl);
    }
    std::cout << "Client disconnected!" << std::endl;
}

void WebSocketServer::on_message(websocketpp::connection_hdl hdl, server::message_ptr msg) {
    std::string payload = msg->get_payload();

    // Assume the message is a symbol subscription, e.g., "subscribe:tBTCUSD"
    if (payload.find("subscribe:") == 0) {
        std::string symbol = payload.substr(10);  // Extract the symbol after "subscribe:"

        std::lock_guard<std::mutex> lock(m_mutex);
        // Add the client to the subscription list for the symbol
        m_subscriptions[symbol].insert(hdl);
        std::cout << "Client subscribed to " << symbol << std::endl;

        // Optionally send an immediate response to confirm subscription
        m_server.send(hdl, "Subscribed to " + symbol, websocketpp::frame::opcode::text);
    }
}

void WebSocketServer::send_orderbook_update(const std::string& symbol, const std::string& orderbook_data) {
    // Compress the orderbook_data
    std::string compressed_data;
    try {
        compressed_data = compress_string(orderbook_data);
    } catch (const std::exception& e) {
        std::cerr << "Compression failed: " << e.what() << std::endl;
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_subscriptions.find(symbol);
    if (it != m_subscriptions.end()) {
        // Broadcast the compressed orderbook data as binary to all subscribed clients
        for (auto& hdl : it->second) {
            websocketpp::frame::opcode::value op = websocketpp::frame::opcode::binary;
            m_server.send(hdl, compressed_data, op);
        }
    }
}

void WebSocketServer::stop() {
    std::cout << "Stopping WebSocket server..." << std::endl;
    m_server.stop_listening();
    m_server.stop();
    std::cout << "WebSocket server stopped." << std::endl;
}
