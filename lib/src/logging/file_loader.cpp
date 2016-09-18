#include <fstream>
#include <logging/file_loader.h>
#include <sstream>

namespace game_engine {
namespace logging {
void
load_policies_from_file( std::string path )
{
  std::ifstream ifstr( path );
  std::string line;
  while ( std::getline( ifstr, line ) ) {
    try {
      load_policies_from_string( line );
    } catch ( const parse_exception& e ) {
      set_policy_for_file_line( __FILE__, __LINE__ + 1, policy::enable );
      LOG( ) << e.what( );
    }
  }
}

void
load_policies_from_string( const std::string& txt )
{
  std::istringstream ss( txt );
  while ( ss.good( ) ) {
    std::string command;
    std::string file;
    unsigned int line;

    ss >> command >> file >> line;
    if ( !ss ) {
      std::ostringstream error_ss;
      error_ss << "error when reading: " << txt;
      throw parse_exception( error_ss.str( ) );
    }

    policy pol;
    if ( command == "enable" ) {
      pol = policy::enable;
    } else if ( command == "disable" ) {
      pol = policy::disable;
    } else {
      std::ostringstream error_ss;
      error_ss << "error when reading (unknown policy): " << txt;
      throw parse_exception( error_ss.str( ) );
    }

    set_policy_for_file_line( file, line, pol );
  }
}
}
}
