#pragma once
// Windows-only shim: maps boost::asio -> standalone Asio headers fetched by CMake.
// On macOS/Linux this file is never on the include path; system Boost is used instead.
#ifndef ASIO_STANDALONE
#  define ASIO_STANDALONE
#endif
#include <asio.hpp>
namespace boost {
  namespace asio = ::asio;
  namespace system {
    using error_code     = std::error_code;
    using error_category = std::error_category;
  }
}
