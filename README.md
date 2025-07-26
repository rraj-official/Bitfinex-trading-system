# Bitfinex Trading System
#### Created by - Rohit Raj

Welcome to the **Bitfinex Trading System**, a high-performance C++ application designed for paper trading on the Bitfinex platform. This system leverages WebSocket communication to provide real-time order book updates, ensuring traders have access to the latest market data with minimal latency.

## About Me

- **Upcoming SWE Intern (Summer 2025) @ Cisco:** Focused on cutting-edge backend development using C++.
- **Full-Stack Developer @ Universal Group** Led the complete platform overhaul utilizing the MERN stack.
- **Web Developer @ Aural:** Led the migration of legacy systems to a modern MERN stack architecture.
- **Education:** Third year CS undergrad @ BITS Pilani
## Features

- **Place Buy/Sell Orders:** Execute spot and futures/options trading with customizable order types.
- **Modify and Cancel Orders:** Easily adjust or cancel existing orders as market conditions change.
- **Real-Time Order Book:** Receive up-to-the-millisecond order book updates via WebSockets.
- **View Current Positions:** Monitor your active trades and positions seamlessly.
- **WebSocket Client Integration:** Connect and subscribe to specific trading symbols for targeted data
- **Modularized Code:** Built with a clean, modular architecture to facilitate scalability, easy maintenance, and future feature expansions. streams.
- **Low-Latency Optimizations:** Engineered for speed and efficiency to support high-frequency trading strategies.

## Low Latency Features

To ensure the Bitfinex Trading System operates with optimal speed and efficiency, several low-latency features have been meticulously implemented:

### 1. Reduced Sleep Interval in Orderbook Updates

**What Was Implemented:**

- **Modification:** Reduced the sleep duration in the `orderbook_update_task` function from **1 second** to **100 milliseconds**.

    ```cpp
    // Original sleep duration
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Updated sleep duration for faster updates
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ```

**How It Helps:**

- **Faster Data Refresh:** By decreasing the interval between successive order book fetches, the application retrieves the latest market data more frequently. This ensures that clients receive up-to-date information with minimal delay.
  
- **Improved Responsiveness:** In volatile markets, prices and order books can change rapidly. A shorter sleep interval allows the system to capture and disseminate these changes in near real-time, enhancing the trading experience for users.
  
- **Competitive Advantage:** Traders relying on timely data can make more informed decisions. Faster updates can be crucial for high-frequency trading strategies where milliseconds can impact profitability.

---

### 2. Multithreading for Concurrent Operations

**What Was Implemented:**

- **Separate Threads:**
  - **WebSocket Server Thread:** Runs the WebSocket server independently to handle client connections without blocking the main application.
  - **Orderbook Update Threads:** Each symbol subscription initiates its own thread for fetching and broadcasting order book updates.

    ```cpp
    // Starting WebSocket server in a separate thread
    std::thread ws_thread([&ws_server]() {
        ws_server.run(9002);  // WebSocket server listens on port 9002
    });
    
    // Starting orderbook update task in a separate thread
    std::thread update_thread(orderbook_update_task, std::ref(ws_server), symbol);
    ```

**How It Helps:**

- **Parallel Processing:** By leveraging multiple threads, the application can perform several tasks simultaneously. The WebSocket server can manage client communications while order book updates proceed in parallel, ensuring that neither process hinders the other.
  
- **Enhanced Throughput:** Multithreading allows the system to handle multiple symbol subscriptions and client connections efficiently, catering to a larger user base without performance degradation.
  
- **Responsiveness:** The main thread remains free to handle user inputs and other tasks without waiting for network operations or data fetching to complete.

---

### 3. Atomic Flags for Thread Synchronization

**What Was Implemented:**

- **Atomic Boolean Flag:** Introduced `std::atomic<bool> stop_updates;` to control the lifecycle of order book update threads safely.

    ```cpp
    std::atomic<bool> stop_updates;
    ```

- **Graceful Termination:** The `wait_for_enter` function sets `stop_updates` to `true` upon pressing Enter, signaling the update thread to cease operations.

    ```cpp
    void wait_for_enter() {
        std::cin.get();  // Wait for the Enter key press
        stop_updates = true;  // Signal to stop updates
    }
    ```

**How It Helps:**

- **Thread Safety:** `std::atomic` ensures that read and write operations on `stop_updates` are thread-safe, preventing race conditions and ensuring reliable communication between threads.
  
- **Immediate Responsiveness:** Users can promptly stop order book updates by pressing Enter, without waiting for the current sleep cycle to finish. This allows for quick transitions back to the main menu or other functionalities.
  
- **Resource Management:** Properly signaling threads to stop prevents resource leaks and ensures that threads terminate cleanly, maintaining the application's stability.

---

### 4. Efficient WebSocket Communication Using `websocketpp`

**What Was Implemented:**

- **WebSocket Server with `websocketpp`:** Utilized the `websocketpp` library to create a robust WebSocket server capable of handling multiple client connections and broadcasting messages efficiently.

    ```cpp
    // Initialize WebSocket server
    WebSocketServer ws_server;
    
    // Handle client subscriptions and messaging
    void WebSocketServer::on_message(websocketpp::connection_hdl hdl, server::message_ptr msg) {
        // Process subscription messages and add clients to subscription lists
    }
    
    // Broadcast order book updates to subscribed clients
    void WebSocketServer::send_orderbook_update(const std::string& symbol, const std::string& orderbook_data) {
        // Iterate through subscribed clients and send updates
    }
    ```

**How It Helps:**

- **Low Overhead Communication:** `websocketpp` is designed for high-performance WebSocket communication, ensuring that messages are transmitted with minimal latency.
  
- **Scalability:** The library efficiently manages multiple concurrent client connections, allowing the server to scale and handle numerous subscriptions without significant performance hits.
  
- **Real-Time Data Delivery:** WebSockets provide a persistent connection between the server and clients, enabling instantaneous data push from the server to clients as soon as updates are available.
  
- **Asynchronous Operations:** `websocketpp` inherently supports asynchronous messaging, ensuring that the server remains responsive even under high load or during intensive data broadcasting.

---

### 5. Efficient Data Structures for Subscription Management

**What Was Implemented:**

- **Subscription Mapping:** Utilized `std::map` and `std::set` to maintain efficient mappings between symbols and subscribed client connections.

    ```cpp
    std::map<std::string, std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>>> m_subscriptions;
    ```

**How It Helps:**

- **Quick Lookup:** `std::map` allows for rapid retrieval of subscription lists based on symbols, facilitating swift broadcasting of updates only to relevant clients.
  
- **Unique Connections:** `std::set` ensures that each client connection is unique within a subscription, preventing duplicate messages and reducing unnecessary network traffic.
  
- **Memory Efficiency:** Efficient data structures minimize memory overhead, allowing the server to handle more subscriptions and client connections without exhausting system resources.

---

### 6. Graceful Shutdown Mechanism

**What Was Implemented:**

- **Thread Synchronization for Shutdown:** Ensured that when the user decides to stop updates or exit the application, all threads are properly synchronized and terminated gracefully.

    ```cpp
    // Waiting for threads to finish before exiting
    enter_thread.join();
    update_thread.join();
    
    // WebSocket server thread
    ws_thread.join();
    ```

**How It Helps:**

- **Preventing Resource Leaks:** Properly joining threads ensures that all resources are freed and no background operations continue after the application is closed.
  
- **Consistent State Management:** Graceful shutdown maintains the application's integrity, ensuring that all operations are completed or halted in a controlled manner, preventing data corruption or inconsistent states.
  
- **User Experience:** Users can exit features smoothly without experiencing abrupt terminations or unexpected behaviors, enhancing the overall usability of the application.

---

### 7. Optimized Order Book Data Handling

**What Was Implemented:**

- **Minimized Data Processing:** Streamlined the way order book data is fetched, processed, and sent to clients to reduce computational overhead and latency.

    ```cpp
    // Simulated fetching of orderbook data
    get_orderbook(symbol);  // Implement efficient orderbook fetching logic
    ```

**How It Helps:**

- **Reduced Computational Load:** Efficient data processing ensures that the system can handle rapid data updates without bottlenecks, maintaining high throughput.
  
- **Faster Data Transmission:** By minimizing the time taken to process and prepare data for transmission, messages are dispatched to clients more swiftly, reducing end-to-end latency.
  
- **Scalability:** Optimized data handling allows the system to manage larger volumes of data and more frequent updates without compromising performance.

---

### Additional Low Latency Features

#### Asynchronous HTTP Requests

- **Implementation:** Use asynchronous libraries or non-blocking I/O for fetching order book data to prevent any potential blocking of threads.
- **Benefit:** Further reduces delays by allowing data fetching to occur in the background without waiting for completion.

#### Data Compression

- **Implementation:** Compress order book data before sending it over WebSockets and decompress it on the client side.
- **Benefit:** Reduces the amount of data transmitted, lowering network latency and improving transmission speed.

---

## Installation & Setup

Follow these steps to set up and run the Bitfinex Trading System on your machine.

### 1. Clone the Repository

```bash

git clone https://github.com/yourusername/bitfinex_trading_system.git
cd bitfinex_trading_system

```
### 2. Install Dependencies

Ensure you have the following dependencies installed on your system:

- **CMake** (version 3.10 or higher)
- **C++ Compiler** supporting C++17
- **CURL**
- **OpenSSL**
- **websocketpp**
- **zlib**

#### On Ubuntu/Debian:

```bash
sudo apt update
sudo apt install -y build-essential cmake libcurl4-openssl-dev libssl-dev libwebsocketpp-dev zlib1g-dev
```

#### On macOS (using Homebrew):

```bash
brew update
brew install cmake curl openssl websocketpp zlib
```

### 3. Build the Project

```bash
mkdir -p build
cd build
cmake ..
make
```

### 4. Run the Application

```bash
./bitfinex_trader_system
```

## Usage

Upon running the application, you'll be presented with a menu to interact with the trading system:

```bash
==============================================
||  Welcome to the Bitfinex Trading System  ||
==============================================
Created by: Rohit Raj
Purpose: For paper trading on the Bitfinex platform.
==============================================

Choose an option below to get started:
-----------------------------------------
1. 📈  Place Buy Order
2. 📉  Place Sell Order
3. 🛠️  Modify Order
4. ❌  Cancel Order
5. 📊  Get Order Book
6. 📑  View Current Positions
7. 🧬  Connect WebSocket Client and Subscribe to Symbol
0. 🚪  Exit
-----------------------------------------
Enter your choice:
```
### Features Overview:

1. **Place Buy/Sell Orders:** Enter the required details to execute buy or sell orders.
2. **Modify/Cancel Orders:** Adjust or cancel existing orders by providing the order ID.
3. **Get Order Book:** Retrieve the current order book for a specified trading symbol.
4. **View Current Positions:** Monitor your active trades and positions.
5. **Connect WebSocket Client:** Subscribe to specific trading symbols to receive real-time order book updates.
6. **Exit:** Gracefully terminate the application.

## Configuration

- **WebSocket Server Port:** The WebSocket server runs on port `9002` by default. You can modify this in the `main.cpp` file if needed.

## Acknowledgements

- **[websocketpp](https://github.com/zaphoyd/websocketpp):** A C++ WebSocket client/server library.
- **[zlib](https://zlib.net/):** A massively spiffy yet delicately unobtrusive compression library.
- **Bitfinex API:** For providing access to trading and market data.