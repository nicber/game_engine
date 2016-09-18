#pragma once

#include "aio.h"
#include <boost/filesystem/path.hpp>

namespace game_engine {
namespace aio {
using path = boost::filesystem::path;

enum class file_access
{
  read_only,
  read_write,
  write_only,
  write_append,
  read_write_append
};

enum class file_mode
{
  create_or_open,
  create_or_truncate,
  truncate,
  open_existing,
};

class file;

#define FILE_FAILURE( X )                                                                                    \
  struct file_##X##_failure : aio_runtime_error                                                              \
  {                                                                                                          \
    using aio_runtime_error::aio_runtime_error;                                                              \
  };

FILE_FAILURE( open );
FILE_FAILURE( close );
FILE_FAILURE( read );
FILE_FAILURE( write );
FILE_FAILURE( unlink );
FILE_FAILURE( truncate );
FILE_FAILURE( stat );
FILE_FAILURE( fstat );
FILE_FAILURE( lstat );

#undef FILE_FAILURE

aio_operation< file > open( path& p, file_access access, file_mode mode );

struct read_result
{
  aio_buffer buf;
  ssize_t read_total = 0;
};

aio_operation< read_result > read( file& file, size_t quantity, int64_t offset );

struct write_result
{
  ssize_t total_written = 0;
};

aio_operation< write_result > write( file& file, aio_buffer buf, int64_t offset );

aio_operation< void > truncate( file& file, int64_t offset );

aio_operation< void > close( file& );

aio_operation< void > unlink( path& p );

using stat_result = uv_stat_t;

aio_operation< stat_result > stat( path& p );
aio_operation< stat_result > fstat( file& file );
aio_operation< stat_result > lstat( path& p );

template < typename T, typename U >
struct fcb_req_wrapper;

class file
{
public:
  struct file_control_block;

  file( file&& )      = default;
  file( const file& ) = delete;
  file& operator=( file&& ) = default;
  file& operator=( const file& ) = delete;

  ~file( );

private:
  using fcb_shr_ptr = std::shared_ptr< file_control_block >;
  file( fcb_shr_ptr fcb_ptr );

  friend aio_operation< file > open( path& p, file_access access, file_mode mode );
  friend aio_operation< read_result > read( file& file, size_t quantity, int64_t offset );
  friend aio_operation< write_result > write( file& file, aio_buffer buf, int64_t offset );
  friend aio_operation< void > truncate( file& file, int64_t offset );
  friend aio_operation< void > close( file& );
  friend aio_operation< stat_result > fstat( file& file );

  template < typename T, typename U >
  friend struct fcb_req_wrapper;

private:
  std::shared_ptr< file_control_block > cblock;
};
}
}
