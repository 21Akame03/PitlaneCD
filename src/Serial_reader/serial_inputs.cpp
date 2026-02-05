#include "serial_inputs.hpp"

#include "boost/system/error_code.hpp"
#include <boost/asio.hpp>
#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

/*
 * Purpose:
 * starts a worker thread to read serial data and parse it
 * the data is then stored in structs and used in the UI
 */
namespace SERIAL {

SerialReader::SerialReader() : running_(false) {}
SerialReader::~SerialReader() { Stop(); }

/*
 * Purpose: Start the SerialReader Worker thread
 * Input: portname, baudrate
 * Output: NONE
 */
bool SerialReader::Start(const std::string &portname, unsigned int baudrate) {
  // avoid Double start
  bool expected = false;
  if (!running_.compare_exchange_strong(expected, true))
    return false;
  // Start the Serial Reader
  serial_thread_ = std::thread(&SerialReader::Run, this, portname, baudrate);

  return true;
}

/*
 * Purpose: Reads serial data and stores in a buffer
 * Input: boost::asio::serial_port
 * Output: String
 */
std::string SerialReader::readFromSerialPort(boost::asio::serial_port &serial) {
  // to store incoming data
  char buffer[100];
  boost::system::error_code ec;

  // read data
  size_t len = boost::asio::read(serial, boost::asio::buffer(buffer), ec);

  if (ec) {
    std::lock_guard<std::mutex> lk(error_mtx_);
    last_error_ = "read: " + ec.message();
    return "";
  }

  return std::string(buffer, len);
}

/*
 * Purpose: Return the data buffer from SerialReader to the UI
 */
std::deque<std::string> SerialReader::PollRxBuffer() {
  std::lock_guard<std::mutex> lk(rx_mtx_);
  std::deque<std::string> out;
  out.swap(rx_buffer_);
  return out;
}

/*
 * Purpose: Return the last error
 */
std::string SerialReader::ConsumeLastError() {
  std::lock_guard<std::mutex> lk(error_mtx_);
  std::string e = last_error_;
  last_error_.clear();
  return e;
}

/*
 * Purpose: Reads serial data and stores in a buffer
 * Input: NONE
 * Output: NONE
 */
void SerialReader::Run(std::string portname, unsigned int baudrate) {
  try {
    boost::asio::io_context io;
    boost::asio::serial_port serial(io);

    boost::system::error_code ec;
    serial.open(portname, ec);

    if (ec) {
      std::lock_guard<std::mutex> lk(error_mtx_);
      last_error_ = "open: " + ec.message();
      running_.store(false);
      return;
    }

    // Configure baudrate
    serial.set_option(boost::asio::serial_port_base::baud_rate(baudrate), ec);
    if (ec) {
      std::lock_guard<std::mutex> lk(error_mtx_);
      last_error_ = "baudrate: " + ec.message();
      running_.store(false);
      return;
    }

    // Good practice is to read line by line
    boost::asio::streambuf sb;
    while (running_.load(std::memory_order_acquire)) {
      std::size_t n = boost::asio::read_until(serial, sb, '\n', ec);
      (void)n; // suppress unused variable warning

      if (!running_.load(std::memory_order_acquire))
        break;
      if (ec) {
        std::lock_guard<std::mutex> lk(error_mtx_);
        last_error_ = "read: " + ec.message();
      }

      // Extract the line from Streambuf
      std::istream is(&sb);
      std::string line;
      std::getline(is, line); // removes '\n'

      {
        std::lock_guard<std::mutex> lk(rx_mtx_); // lock the buffer
        rx_buffer_.push_back(std::move(line));

        // limit queue to 2000
        if (rx_buffer_.size() > 2000) {
          rx_buffer_.pop_front();
        }
      }
    }
  } catch (std::exception &e) {
    std::lock_guard<std::mutex> lk(error_mtx_);
    std::cout << "Exception: " << e.what() << "\n";
  }
}

/*
 * Purpose: Stop the serial reader
 */
void SerialReader::Stop() {
  if (!running_.exchange(false))
    return;
  if (serial_thread_.joinable()) {
    serial_thread_.join();
  }
}

/*
 * Purpose: Returns the running state of the Serial reader worker
 */
bool SerialReader::IsRunning() const {
  return running_.load(std::memory_order_acquire);
}

} // namespace SERIAL
