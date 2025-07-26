// include/websocket_server.hpp
#ifndef WEBSOCKET_SERVER_HPP
#define WEBSOCKET_SERVER_HPP

#include <string>
#include <set>
#include <map>
#include <mutex>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

class WebSocketServer {
public:
    WebSocketServer();
    void run(uint16_t port);
    void send_orderbook_update(const std::string& symbol, const std::string& orderbook_data);
    void stop();

private:
    typedef websocketpp::server<websocketpp::config::asio> server;

    server m_server;
    std::map<std::string, std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>>> m_subscriptions;
    std::mutex m_mutex; // Mutex for thread-safe access to subscriptions

    void on_open(websocketpp::connection_hdl hdl);
    void on_close(websocketpp::connection_hdl hdl);
    void on_message(websocketpp::connection_hdl hdl, server::message_ptr msg);
};
#endif // WEBSOCKET_SERVER_HPP
