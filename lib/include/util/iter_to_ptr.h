#pragma once

#include <type_traits>
#include <vector>

namespace game_engine {
namespace util {
template < typename T >
struct iter_to_ptr
{
public:
  iter_to_ptr( T* begin_, T* end_ ) : begin( begin_ ), end( end_ )
  {
  }

  iter_to_ptr( typename std::vector< std::remove_const_t< T > >::const_iterator begin_,
               typename std::vector< std::remove_const_t< T > >::const_iterator end_ )
  {
    begin = &*begin_;
    end   = &*end_;
  }

  template < typename U >
  iter_to_ptr( U begin_, U end_ )
  {
    auto size = std::distance( begin_, end_ );
    copy_vec.reserve( size );
    std::copy( begin_, end_, std::back_inserter( copy_vec ) );
    begin = copy_vec.data( );
    end   = begin + copy_vec.size( );
  }

  const T* begin;
  const T* end;

private:
  std::vector< T > copy_vec;
};

template < typename It >
iter_to_ptr< typename std::iterator_traits< It >::value_type >
trans_iter_to_ptr( It beg, It end )
{
  return { beg, end };
}
}
}
