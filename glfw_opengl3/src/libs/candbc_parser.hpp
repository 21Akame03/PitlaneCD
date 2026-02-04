#ifndef CANDBC_PARSER_HPP
#define CANDBC_PARSER_HPP

#include <memory>
#include <string>
#include "dbcppp/Network.h"

namespace CANDBC_PARSER {

// Load a DBC file into an INetwork; throws std::runtime_error on failure.
std::unique_ptr<dbcppp::INetwork> load_dbc(const std::string& filepath);

} // namespace CANDBC_PARSER


#endif
