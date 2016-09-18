#include "logging/control_log.h"
#include <atomic>
#include <boost/functional/hash.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <unordered_map>

namespace game_engine {
namespace logging {
bool
enabled( const char* file, unsigned int line )
{
  auto pol = get_policy_for( file, line );
  return pol == policy::enable;
}

static std::atomic< bool > iterating( false );
static policy default_policy = policy::disable;
using line_map               = std::unordered_map< unsigned int, policy >;
static std::unordered_map< std::string, line_map > file_line_policies = {
  { "../lib/src/thr_queue/global_thr_pool_impl.cpp", { { 0, policy::disable } } },
  { "../lib/src/thr_queue/global_thr_pool_impl_linux.cpp", { { 0, policy::disable } } }
};

static boost::recursive_mutex policy_mutex;

static void
check_not_iterating( )
{
  if ( iterating ) {
    throw std::logic_error( "can't modify logging policies while iterating" );
  }
}

void
set_default_for_all( policy pol )
{
  check_not_iterating( );
  default_policy = pol;
}

void
set_default_for_file( std::string file, policy pol )
{
  check_not_iterating( );
  boost::lock_guard< boost::recursive_mutex > l( policy_mutex );
  file_line_policies[ file ][ 0 ] = pol;
}

void
set_policy_for_file_line( std::string file, unsigned int line, policy pol )
{
  check_not_iterating( );
  boost::lock_guard< boost::recursive_mutex > l( policy_mutex );
  file_line_policies[ file ][ line ] = pol;
}

policy
get_policy_for( const std::string& file, unsigned int line )
{
  boost::lock_guard< boost::recursive_mutex > l( policy_mutex );
  auto line_map_it = file_line_policies.find( file );
  if ( line_map_it != file_line_policies.end( ) ) {
    auto& line_map = line_map_it->second;
    auto it        = line_map.find( line );
    if ( it != line_map.end( ) ) {
      return it->second;
    } else {
      auto file_default_it = line_map.find( 0 );
      if ( file_default_it != line_map.end( ) ) {
        return file_default_it->second;
      }
    }
  }
  return default_policy;
}

policy
get_policy_for_file( const std::string& file )
{
  boost::lock_guard< boost::recursive_mutex > l( policy_mutex );
  auto line_map_it = file_line_policies.find( file );
  if ( line_map_it != file_line_policies.end( ) ) {
    auto& line_map       = line_map_it->second;
    auto file_default_it = line_map.find( 0 );
    if ( file_default_it != line_map.end( ) ) {
      return file_default_it->second;
    }
  }

  return default_policy;
}

policy
get_default_policy( )
{
  boost::lock_guard< boost::recursive_mutex > l( policy_mutex );
  return default_policy;
}

void
remove_file_policy( const std::string& file )
{
  remove_file_line_policy( file, 0 );
}

void
remove_file_line_policy( const std::string& file, unsigned int line )
{
  check_not_iterating( );
  boost::lock_guard< boost::recursive_mutex > l( policy_mutex );
  auto line_map_it = file_line_policies.find( file );
  if ( line_map_it != file_line_policies.end( ) ) {
    auto& line_map = line_map_it->second;
    line_map.erase( line );
  }
}

void
remove_all_policies( )
{
  check_not_iterating( );
  boost::lock_guard< boost::recursive_mutex > l( policy_mutex );
  file_line_policies = {};
}

applied_policy::applied_policy( const std::string& f, unsigned int l, policy p )
  : file( f ), line( l ), pol( p )
{
}

bool
operator==( const applied_policy& lhs, const applied_policy& rhs )
{
  return lhs.line == rhs.line && lhs.pol == rhs.pol && lhs.file == rhs.file;
}

bool
operator!=( const applied_policy& lhs, const applied_policy& rhs )
{
  return !( lhs == rhs );
}

void
apply_to_all_policies( visitor_func vf )
{
  boost::lock_guard< boost::recursive_mutex > l( policy_mutex );
  iterating = true;
  try {
    for ( auto& p1 : file_line_policies ) {
      applied_policy ap_pol( p1.first, 0, policy::disable );
      for ( auto& p2 : p1.second ) {
        ap_pol.line = p2.first;
        ap_pol.pol  = p2.second;
        vf( std::move( ap_pol ) );
      }
    }

    const static std::string empty_str;
    applied_policy ap_pol( empty_str, 0, default_policy );
    vf( std::move( ap_pol ) );
  } catch ( ... ) {
    iterating = false;
    throw;
  }
  iterating = false;
}
}
}
