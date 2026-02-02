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
  void configureSerialPort(boost::asio::serial_port &serial,
                           const std::string &portname, unsigned int baudrate);
  std::string readFromSerialPort(boost::asio::serial_port &serial);

  // called from UI
  std::deque<std::string> PollRxBuffer();

  // called from UI: get and clear last error
  std::string ConsumeLastError();

private:
  std::atomic<bool> running_;

  void ReaderLoop(std::string portname, unsigned int baudrate);
  std::thread serial_thread_;

  // producer (worker ) -> consumer(UI)
  std::mutex rx_mtx_;
  std::deque<std::string> rx_buffer_;

  // Error reporting (Worker sets, UI read)
  std::mutex error_mtx_;
  std::string last_error_;
};

} // namespace SERIAL

// Serial Port Discovery
std::vector<std::string> listSerialPorts();

#endif // SERIAL_INPUTS_HPP
