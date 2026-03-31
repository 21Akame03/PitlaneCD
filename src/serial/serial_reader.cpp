#include "serial/serial_reader.hpp"

#include "boost/system/error_code.hpp"
#include <boost/asio.hpp>
#include <functional>
#include <iostream>
#include <thread>

namespace serial {

SerialReader::SerialReader() : running_(false) {}
SerialReader::~SerialReader() { Stop(); }

bool SerialReader::Start(const std::string &portname, unsigned int baudrate) {
  bool expected = false;
  if (!running_.compare_exchange_strong(expected, true))
    return false;
  serial_thread_ = std::thread(&SerialReader::Run, this, portname, baudrate);
  return true;
}

std::deque<std::string> SerialReader::PollRxBuffer() {
  std::lock_guard<std::mutex> lk(rx_mtx_);
  std::deque<std::string> out;
  out.swap(rx_buffer_);
  return out;
}

std::string SerialReader::ConsumeLastError() {
  std::lock_guard<std::mutex> lk(error_mtx_);
  std::string e = last_error_;
  last_error_.clear();
  return e;
}

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

    serial.set_option(boost::asio::serial_port_base::baud_rate(baudrate), ec);
    if (ec) {
      std::lock_guard<std::mutex> lk(error_mtx_);
      last_error_ = "baudrate: " + ec.message();
      running_.store(false);
      return;
    }

    {
      std::lock_guard<std::mutex> lk(serial_mtx_);
      active_serial_ = &serial;
      active_io_ = &io;
    }

    boost::asio::streambuf sb;

    std::function<void()> do_read;
    do_read = [&]() {
      boost::asio::async_read_until(
          serial, sb, '\n',
          [&](boost::system::error_code ec, std::size_t) {
            if (ec || !running_.load(std::memory_order_acquire)) {
              if (ec && ec != boost::asio::error::operation_aborted) {
                std::lock_guard<std::mutex> lk(error_mtx_);
                last_error_ = "read: " + ec.message();
              }
              return;
            }

            std::istream is(&sb);
            std::string line;
            std::getline(is, line);

            {
              std::lock_guard<std::mutex> lk(rx_mtx_);
              rx_buffer_.push_back(std::move(line));
              if (rx_buffer_.size() > 2000) {
                rx_buffer_.pop_front();
              }
            }

            do_read();
          });
    };

    do_read();
    io.run();

    {
      std::lock_guard<std::mutex> lk(serial_mtx_);
      active_serial_ = nullptr;
      active_io_ = nullptr;
    }
  } catch (std::exception &e) {
    {
      std::lock_guard<std::mutex> lk(serial_mtx_);
      active_serial_ = nullptr;
      active_io_ = nullptr;
    }
    {
      std::lock_guard<std::mutex> lk(error_mtx_);
      last_error_ = "Exception: " + std::string(e.what());
    }
  }
}

void SerialReader::Send(const std::string &data) {
  std::lock_guard<std::mutex> lk(serial_mtx_);
  if (!active_io_ || !active_serial_ || !running_.load(std::memory_order_acquire))
    return;

  auto buf = std::make_shared<std::string>(data);
  boost::asio::post(*active_io_, [this, buf]() {
    boost::system::error_code ec;
    boost::asio::write(*active_serial_, boost::asio::buffer(*buf), ec);
    if (ec) {
      std::lock_guard<std::mutex> lk(error_mtx_);
      last_error_ = "write: " + ec.message();
    }
  });
}

void SerialReader::Stop() {
  if (!running_.exchange(false))
    return;
  {
    std::lock_guard<std::mutex> lk(serial_mtx_);
    if (active_serial_) {
      boost::system::error_code ec;
      active_serial_->cancel(ec);
    }
    if (active_io_) {
      active_io_->stop();
    }
  }
  if (serial_thread_.joinable()) {
    serial_thread_.join();
  }
}

bool SerialReader::IsRunning() const {
  return running_.load(std::memory_order_acquire);
}

} // namespace serial
