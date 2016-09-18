#pragma once

#include <stdexcept>
#include <string>

namespace game_engine {
namespace logging {
class parse_exception : public std::runtime_error
{
  using std::runtime_error::runtime_error;
};

void load_policies_from_file( std::string path );

void load_policies_from_string( const std::string& txt );

template < typename It >
void load_policies_line_by_line( It begin, It end );
}
}

#include "file_loader.inl"
