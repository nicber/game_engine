#include "thr_queue/queue.h"
#include "thr_queue/coroutine.h"
#include "thr_queue/event/cond_var.h"
#include "thr_queue/event/mutex.h"
#include "thr_queue/global_thr_pool.h"
#include "gtest/gtest.h"
#include <algorithm>
#include <boost/context/all.hpp>
#include <iostream>

#include "../src/thr_queue/event/uv_thread.h"
#include "thr_queue/util_queue.h"

#include <boost/chrono.hpp>

TEST( ThrQueue, ExecutesCode )
{
  using namespace game_engine::thr_queue;
  queue q( queue_type::parallel );
  std::atomic< int > count( 0 );

  for ( size_t i = 0; i < 10; ++i ) {
    q.submit_work( [&]( ) { count++; } );
  }

  schedule_queue( std::move( q ) );

  // sleep to let the queue be executed.
  boost::chrono::milliseconds dura( 50 );
  boost::this_thread::sleep_for( dura );

  EXPECT_EQ( 10, count );
}

TEST( ThrQueue, ParallelQinSerial )
{
  using namespace game_engine::thr_queue;
  queue q_ser( queue_type::serial );
  queue q_par( queue_type::parallel );
  std::atomic< int > count( 0 );

  for ( size_t i = 0; i < 10; ++i ) {
    q_par.submit_work( [&] { count++; } );
  }

  q_ser.append_queue( std::move( q_par ) );

  auto fut = q_ser.submit_work( [] {} );

  schedule_queue( std::move( q_ser ) );

  fut.wait( );

  EXPECT_EQ( 10, count );
}

TEST( ThrQueue, SerialQinParallel )
{
  using namespace game_engine::thr_queue;
  queue q_par( queue_type::parallel );
  boost::mutex mt;
  boost::condition_variable cv;
  std::atomic< int > count[ 100 ];
  std::vector< int > counts[ 100 ];
  std::atomic< int > done( 0 );

  std::generate( std::begin( count ), std::end( count ), [] { return 0; } );

  for ( size_t i = 0; i < 100; ++i ) {
    queue q_ser( queue_type::serial );
    counts[ i ].reserve( 100 );
    for ( size_t j = 0; j < 100; ++j ) {
      q_ser.submit_work( [&, i] {
        auto val = ++count[ i ];
        counts[ i ].push_back( val );
      } );
    }

    q_ser.submit_work( [&] {
      boost::lock_guard< boost::mutex > lock( mt );
      ++done;
      cv.notify_one( );
    } );

    q_par.append_queue( std::move( q_ser ) );
  }

  boost::unique_lock< boost::mutex > lock( mt );

  schedule_queue( std::move( q_par ) );

  while ( done < 100 ) {
    cv.wait( lock );
  }

  for ( int val : count ) {
    EXPECT_EQ( 100, val );
  }

  for ( auto& vec : counts ) {
    int prev_step = 0;
    for ( int step : vec ) {
      EXPECT_EQ( prev_step + 1, step );
      prev_step = step;
    }
  }
}

TEST( ThrQueue, DefaultParQueue )
{
  std::atomic< int > count( 0 );
  std::atomic< bool > done( false );
  boost::mutex mt;
  boost::condition_variable cv;
  boost::unique_lock< boost::mutex > lock( mt );

  for ( size_t i = 0; i < 10000; ++i ) {
    game_engine::thr_queue::default_par_queue( ).submit_work( [&] {
      if ( ++count == 10000 ) {
        boost::lock_guard< boost::mutex > lock( mt );
        done = true;
        cv.notify_one( );
      }
    } );
  }

  while ( !done ) {
    cv.wait( lock );
  }

  EXPECT_EQ( 10000, count );
}

TEST( ThrQueue, FillStack )
{
// boost bug #12340 doesn't let us grow the stack.
#ifndef _WIN32
  game_engine::thr_queue::default_par_queue( )
    .submit_work( [] {
      uint8_t test[ 100 * 4096 ];
      for ( size_t i = 0; i < sizeof( test ); ++i ) {
        test[ sizeof( test ) - i ] = 0;
      }
    } )
    .wait( );
#endif
}

TEST( ThrQueue, SerQueue )
{
  const int n_tasks = 10000;
  auto q_ser        = game_engine::thr_queue::ser_queue( );
  boost::mutex mt;
  boost::condition_variable cv;
  boost::unique_lock< boost::mutex > lock( mt );
  // we use atomics to be sure that a possible bug in the implementation isn't
  // "hiding" itself by means of a race condition.
  std::atomic< int > last_exec( -1 );
  std::atomic< bool > correct_order( true );
  std::atomic< bool > done( false );

  for ( int i = 0; i < n_tasks; ++i ) {
    q_ser.submit_work( [&, i] {
      if ( last_exec.exchange( i ) != i - 1 ) {
        correct_order = false;
      }
      if ( i == n_tasks - 1 ) {
        boost::lock_guard< boost::mutex > lock( mt );
        done = true;
        cv.notify_one( );
      }
    } );
  }

  cv.wait( lock, [&]( ) -> bool { return done; } );

  EXPECT_EQ( n_tasks - 1, last_exec );
  EXPECT_EQ( true, correct_order );
}

TEST( ThrQueue, SerQueueAllAtOnce )
{
  const int n_tasks = 10000;
  game_engine::thr_queue::queue q_ser( game_engine::thr_queue::queue_type::serial );

  boost::mutex mt;
  boost::condition_variable cv;
  boost::unique_lock< boost::mutex > lock( mt );
  // we use atomics to be sure that a possible bug in the implementation isn't
  // "hiding" itself by means of a race condition.
  std::atomic< int > last_exec( -1 );
  std::atomic< bool > correct_order( true );
  std::atomic< bool > done( false );

  for ( int i = 0; i < n_tasks; ++i ) {
    q_ser.submit_work( [&, i] {
      if ( last_exec.exchange( i ) != i - 1 ) {
        correct_order = false;
      }
      if ( i == n_tasks - 1 ) {
        boost::lock_guard< boost::mutex > lock( mt );
        done = true;
        cv.notify_one( );
      }
    } );
  }

  game_engine::thr_queue::schedule_queue( std::move( q_ser ) );
  cv.wait( lock, [&]( ) -> bool { return done; } );

  EXPECT_EQ( n_tasks - 1, last_exec );
  EXPECT_EQ( true, correct_order );
}

TEST( ThrQueue, LockUnlock )
{
  using e_mutex = game_engine::thr_queue::event::mutex;
  using game_engine::thr_queue::queue;
  using game_engine::thr_queue::queue_type;
  queue q_par( queue_type::parallel );

  e_mutex mt;
  bool done = false;
  boost::mutex cv_mt;
  boost::condition_variable cv;
  boost::unique_lock< boost::mutex > lock( cv_mt );

  struct
  {
    int a = 0;
    int b = 0;
  } test;

  for ( size_t i = 0; i < 10000; i++ ) {
    q_par.submit_work( [&] {
      boost::lock_guard< e_mutex > lock( mt );
      test.a++;
      test.b++;
    } );
  }

  queue q_ser( queue_type::serial );
  q_ser.append_queue( std::move( q_par ) );
  q_ser.submit_work( [&] {
    boost::lock_guard< boost::mutex > lock_lamb( cv_mt );
    done = true;
    cv.notify_one( );
  } );

  schedule_queue( std::move( q_ser ) );

  cv.wait( lock, [&] { return done; } );

  EXPECT_EQ( 10000, test.a );
  EXPECT_EQ( 10000, test.b );
}

TEST( ThrQueue, UvRuns )
{
  using namespace game_engine::thr_queue::event;
  const unsigned int number = 10000;
  std::atomic< unsigned int > counter{ 0 };
  std::vector< future< void > > futures;
  futures.reserve( number );
  for ( unsigned int i = 0; i < number; ++i ) {
    futures.emplace_back( uv_thr_cor_do< void >( [&]( auto prom ) {
      ++counter;
      prom.set_value( );
    } ) );
  }
  wait_all( futures.cbegin( ), futures.cend( ) );
  EXPECT_EQ( number, counter );
}

TEST( ThrQueue, UvWait )
{
  using namespace game_engine::thr_queue;
  using namespace game_engine::thr_queue::event;
  const unsigned int number = 10000;
  std::atomic< unsigned int > counter{ 0 };
  queue q{ queue_type::parallel };
  for ( unsigned int i = 0; i < number; ++i ) {
    q.submit_work( [&] {
      auto fut = uv_thr_cor_do< void >( [&]( auto prom ) {
        ++counter;
        prom.set_value( );
      } );
      fut.wait( );
    } );
  }
  queue q_ser{ queue_type::serial };
  q_ser.append_queue( std::move( std::move( q ) ) );
  auto fut = q_ser.submit_work( [] {} );
  schedule_queue( std::move( q_ser ) );
  fut.wait( );
  EXPECT_EQ( number, counter );
}
