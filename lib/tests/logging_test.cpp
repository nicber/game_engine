#include <gtest/gtest.h>
#include <logging/control_log.h>

using namespace game_engine::logging;

TEST( LoggingTest, HelloWorld )
{
  LOG( ) << "Hello "
         << "World!";
}

TEST( LoggingTest, BasicControl )
{
  set_default_for_all( policy::disable );
  set_default_for_file( "lala", policy::enable );
  EXPECT_EQ( policy::enable, get_policy_for( "lala", 100 ) );
  EXPECT_EQ( policy::enable, get_policy_for( "lala", 10 ) );
  EXPECT_EQ( policy::enable, get_policy_for( "lala", 40 ) );
  EXPECT_EQ( policy::disable, get_policy_for( "bla", 30 ) );
  set_policy_for_file_line( "lala", 300, policy::disable );
  EXPECT_EQ( policy::enable, get_policy_for( "lala", 40 ) );
  EXPECT_EQ( policy::disable, get_policy_for( "lala", 300 ) );
  remove_file_line_policy( "lala", 300 );
  EXPECT_EQ( policy::enable, get_policy_for( "lala", 300 ) );
  EXPECT_EQ( policy::enable, get_policy_for( "lala", 30 ) );
  EXPECT_EQ( policy::disable, get_policy_for( "bla", 40 ) );
  remove_file_policy( "lala" );
  EXPECT_EQ( policy::disable, get_policy_for( "lala", 300 ) );
  EXPECT_EQ( policy::disable, get_policy_for( "lala", 30 ) );
  EXPECT_EQ( policy::disable, get_policy_for( "bla", 304 ) );
  remove_all_policies( );
  int count_policies = 0;
  apply_to_all_policies( [&]( applied_policy ) { count_policies++; } );
  EXPECT_EQ( 1, count_policies );
}

TEST( LoggingTest, ExceptionsDuringIter )
{
  std::vector< applied_policy > policies_before_iter;
  set_policy_for_file_line( "kaka", 500, policy::enable );
  set_default_for_file( "bla", policy::disable );
  set_default_for_file( "kaka", policy::enable );
  set_default_for_file( "kaka", policy::disable );
  set_policy_for_file_line( "hola", 1000, policy::enable );
  apply_to_all_policies( [&]( applied_policy pol ) {
    EXPECT_THROW( remove_file_line_policy( pol.file, pol.line ), std::logic_error );
    EXPECT_THROW( remove_file_policy( pol.file ), std::logic_error );
    EXPECT_THROW( set_default_for_all( policy::enable ), std::logic_error );
    EXPECT_THROW( set_default_for_file( pol.file, policy::enable ), std::logic_error );
    EXPECT_THROW( set_policy_for_file_line( pol.file, pol.line, policy::enable ), std::logic_error );
    EXPECT_NO_THROW( get_default_policy( ) );
    EXPECT_NO_THROW( get_policy_for_file( pol.file ) );
    EXPECT_NO_THROW( get_policy_for( pol.file, pol.line ) );
    policies_before_iter.emplace_back( std::move( pol ) );
  } );
  std::vector< applied_policy > policies_after_iter;
  apply_to_all_policies(
    [&]( applied_policy pol ) { policies_after_iter.emplace_back( std::move( pol ) ); } );
  EXPECT_EQ( policies_before_iter, policies_after_iter );
}
