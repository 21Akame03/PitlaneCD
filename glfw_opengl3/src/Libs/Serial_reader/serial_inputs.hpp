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

  void Run();
  void Stop();
  bool IsRunning() const;
  void configureSerialPort(boost::asio::serial_port &serial,
                           const std::string &portname, unsigned int baudrate);
  std::string readFromSerialPort(boost::asio::serial_port &serial);

private:
  std::atomic<bool> running_;
};

} // namespace SERIAL

#endif // SERIAL_INPUTS_HPP
