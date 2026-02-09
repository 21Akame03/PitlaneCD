#ifndef SERIAL_INPUTS_HPP
#define SERIAL_INPUTS_HPP

#include <atomic>
#include <boost/asio.hpp>
#include <vector>

namespace SERIAL {
class SerialReader {
public:
  SerialReader();
  ~SerialReader();

  bool Start(const std::string &portname, unsigned int baudrate);
  void Run(std::string portname, unsigned int baudrate);
  void Stop();
  bool IsRunning() const;
  // called from UI
  std::deque<std::string> PollRxBuffer();

  // called from UI: get and clear last error
  std::string ConsumeLastError();

private:
  std::atomic<bool> running_;

  std::thread serial_thread_;

  // producer (worker ) -> consumer(UI)
  std::mutex rx_mtx_;
  std::deque<std::string> rx_buffer_;

  // Error reporting (Worker sets, UI read)
  std::mutex error_mtx_;
  std::string last_error_;

  // Active serial port and io_context (for cancellation on Stop)
  std::mutex serial_mtx_;
  boost::asio::serial_port *active_serial_ = nullptr;
  boost::asio::io_context *active_io_ = nullptr;
};

} // namespace SERIAL

// Serial Port Discovery
std::vector<std::string> listSerialPorts();

#endif // SERIAL_INPUTS_HPP
