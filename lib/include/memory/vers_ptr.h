#pragma once

#include <memory>
#include <vector>

namespace game_engine {
template < typename T >
class vers_ptr;

class base_vers_data
{
public:
  using time_point  = unsigned int;
  using value_index = unsigned int;
};

/** \brief This class stores a series of Ts.
 * It's used by the vers_ptr class.
 */
template < typename T, typename Policy >
class vers_data : public base_vers_data, private Policy
{
public:
  using value_type = T;

  vers_data( T val );

  void remove_older_than( time_point limit );
  void remove_newer_than( time_point limit );

  const T& get_latest( ) const;
  const T& get_corresp( time_point time_val ) const;

  time_point get_time_of( value_index index ) const;
  const T& get_value_of( value_index index ) const;

  /** This member function returns a reference to a copy of the latest value
   * and registers it as the latest one if Policy::must_copy(time_point) returns
   * true.
   * Otherwise it just returns a reference to the latest T.
   */
  T& copy_or_get_latest( );

  value_index size( ) const;

  vers_ptr< const vers_data< T, Policy > > ptr( ) const;
  vers_ptr< vers_data< T, Policy > > ptr( );

private:
  using time_and_ptr = std::pair< time_point, std::unique_ptr< T > >;

  /** Guaranteed to be in ascending order with regards to the time_points.
   * That means that newer values have higher indexes.
   * */
  std::vector< time_and_ptr > old_data;

  /** \brief Stores the data returned by latest. */
  std::pair< time_point, T > newest_data;

  /** \brief This member function tests whether the data vector is sorted
   * and it'll check it the newest value is in fact the newest one.
   */
  void test_invariants( ) const;
};

/** \brief This template implements a smart pointer that provides a clean
 * interface to interact with a vers_data object.
 * Everytime it's dereferenced it calls copy_or_get_latest() and returns its
 * result.
 * */
template < typename Data >
class vers_ptr
{
public:
  using value_type = typename Data::value_type;
  vers_ptr( Data& data );

  const value_type& operator*( ) const;
  value_type& operator*( );

  const value_type* operator->( ) const;
  value_type* operator->( );

  /** \brief Returns a pointer to the data. */
  const value_type* get( ) const;
  value_type* get( );

private:
  Data* data;
};

template < typename Data >
class vers_ptr< const Data >
{
public:
  using value_type = typename Data::value_type;
  vers_ptr( const Data& data );

  const value_type& operator*( ) const;
  const value_type* operator->( ) const;

  /** Returns a pointer to the data. */
  const value_type* get( ) const;

private:
  const Data* data;
};
}
#include "vers_ptr.inl"
