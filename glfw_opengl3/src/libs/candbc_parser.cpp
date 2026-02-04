#include "candbc_parser.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <stdexcept>

#include "dbcppp/Message.h"
#include "dbcppp/Network.h"
#include "dbcppp/Signal.h"

namespace CANDBC_PARSER {

// Typical CAN frame structure / Format
typedef struct {
  uint32_t id;
  uint8_t dlc;
  uint8_t __pad;  // Padding
  uint8_t __res0; // Reserved // Padding
  uint8_t __res1; // Reserved // Padding
  uint8_t data[8];
} can_frame_t;

class DBCParser {
public:
  std::string dbcfilepath = "";

  //
  std::unique_ptr<dbcppp::INetwork> net;

  DBCParser(std::string &filepath) { load_dbc(filepath); };

  /*
   * Purpose: Get the network from the DBC file and check if it is present
   * Input: DBC filepath
   * Output: true if successful
   */
  bool load_dbc(const std::string &filepath) {
    this->dbcfilepath = filepath;

    std::ifstream idbc(this->dbcfilepath);
    this->net = dbcppp::INetwork::LoadDBCFromIs(idbc);

    if (net.get() == nullptr) {
      std::cerr << "failed to load dbc" << std::endl;
      return false;
    }

    return true;
  };

  bool decode_message() {

    std::unordered_map<uint64_t, const dbcppp::IMessage *> messages;

    /* For every messages in the DBC file,
     * make a key data pair using the can_message id as key and the message as
     * value
     */

    for (const dbcppp::IMessage &msg : net->Messages()) {
      messages.insert(std::make_pair(msg.Id(), &msg));
    }

    can_frame_t frame;
    while (1) {
      // Receive CAN frame as byte from the ESP32 / CAN_sniffer and decode
    }

    return true;
  }
};
} // namespace CANDBC_PARSER
