#include "serial_inputs.hpp"

#include "imgui.h"
#include <boost/asio.hpp>
#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

namespace SERIAL {

SerialReader::SerialReader() : running_(false) {}

SerialReader::~SerialReader() { Stop(); }

bool errorToastisOn = false;
// While isConnected is false, we will check for serial port
bool isConnected = false;

void SerialReader::configureSerialPort(boost::asio::serial_port &serial,
                                       const std::string &portname,
                                       unsigned int baudrate) {
  // open serial port
  serial.open(portname);
  // Set baud rate
  serial.set_option(boost::asio::serial_port_base::baud_rate(baudrate));
}

std::string SerialReader::readFromSerialPort(boost::asio::serial_port &serial) {
  // to store incoming data
  char buffer[100];
  boost::system::error_code ec;

  // read data
  size_t len = boost::asio::read(serial, boost::asio::buffer(buffer), ec);

  if (ec && !errorToastisOn) {
    ImGui::Begin("Error");
    ImGui::Text("Error: %s", ec.message().c_str());
    ImGui::End();
    errorToastisOn = true;

    return "";
  } else {
    errorToastisOn = false;
  }

  return std::string(buffer, len);
}

void SerialReader::Run() {
  // Implementation for reading serial data
  // Fake data generation for UI design purposes
  // running_.store(true, std::memory_order_release);
  // const auto start = std::chrono::steady_clock::now();
  //
  // while (running_.load(std::memory_order_acquire)) {
  //   const auto now = std::chrono::steady_clock::now();
  //   const float t =
  //       std::chrono::duration<float>(now - start).count(); // seconds since
  //       run
  //
  //   static float xs1[1001], ys1[1001];
  //   for (int i = 0; i < 1001; ++i) {
  //     xs1[i] = i * 0.001f;
  //     ys1[i] = 0.5f + 0.5f * std::sinf(50.0f * (xs1[i] + t / 10.0f));
  //   }
  //   static double xs2[20], ys2[20];
  //   for (int i = 0; i < 20; ++i) {
  //     xs2[i] = i * 1 / 19.0f;
  //     ys2[i] = xs2[i] * xs2[i];
  //   }
  //
  //   std::this_thread::sleep_for(std::chrono::milliseconds(16));
  // }
}

void SerialReader::Stop() { running_.store(false, std::memory_order_release); }

bool SerialReader::IsRunning() const {
  return running_.load(std::memory_order_acquire);
}

} // namespace SERIAL
