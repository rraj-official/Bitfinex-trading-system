# CMakeLists.txt

cmake_minimum_required(VERSION 3.10)
project(BitfinexTraderSystem)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(include)

# Find required packages
find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(websocketpp CONFIG REQUIRED)
find_package(ZLIB REQUIRED)  # Find zlib

# Add executable
add_executable(bitfinex_trader_system
    src/main.cpp
    src/api.cpp
    src/utils.cpp
    src/websocket_server.cpp
)

# Link libraries
target_link_libraries(bitfinex_trader_system
    CURL::libcurl
    OpenSSL::SSL
    OpenSSL::Crypto
    websocketpp::websocketpp
    ZLIB::ZLIB  # Link zlib
)
