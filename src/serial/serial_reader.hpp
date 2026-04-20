#ifndef SERIAL_READER_HPP
#define SERIAL_READER_HPP

#include <atomic>
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <boost/asio.hpp>
#include <vector>

namespace serial {

class SerialReader {
public:
  SerialReader();
  ~SerialReader();

  bool Start(const std::string &portname, unsigned int baudrate);
  void Run(std::string portname, unsigned int baudrate);
  void Stop();
  bool IsRunning() const;
  std::deque<std::string> PollRxBuffer();
  void Send(const std::string &data);
  std::string ConsumeLastError();

private:
  std::atomic<bool> running_;
  std::thread serial_thread_;

  std::mutex rx_mtx_;
  std::deque<std::string> rx_buffer_;

  std::mutex error_mtx_;
  std::string last_error_;

  std::mutex serial_mtx_;
  boost::asio::serial_port *active_serial_ = nullptr;
  boost::asio::io_context *active_io_ = nullptr;
};

std::vector<std::string> list_serial_ports();

} // namespace serial

#endif // SERIAL_READER_HPP
