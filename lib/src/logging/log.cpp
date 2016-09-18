#include "logging/log_priv.h"
#include <sstream>

namespace game_engine {
namespace logging {
logger::logger( const char* function, const char* pretty_function, const char* file, unsigned int line )
  : d( new message )
{
  d->function        = function;
  d->pretty_function = pretty_function;
  d->file            = file;
  d->line            = line;
}

logger::~logger( )
{
  handle_complete_message( std::move( d ) );
}

std::ostream&
logger::get_stream( )
{
  return d->ss;
}
}
}
