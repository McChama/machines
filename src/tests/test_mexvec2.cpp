#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "mathex/vec2.hpp"

TEST_CASE( "MexVec2 construction and accessors", "[mathex][vec2]" )
{
    MexVec2 zero;
    REQUIRE( zero.x() == 0 );
    REQUIRE( zero.y() == 0 );

    MexVec2 v( 3, 4 );
    REQUIRE( v.x() == 3 );
    REQUIRE( v.y() == 4 );
}

TEST_CASE( "MexVec2 equality", "[mathex][vec2]" )
{
    MexVec2 a( 1, 2 );
    MexVec2 b( 1, 2 );
    MexVec2 c( 3, 4 );

    REQUIRE( a == b );
    REQUIRE_FALSE( a == c );
}

TEST_CASE( "MexVec2 isZeroVector", "[mathex][vec2]" )
{
    REQUIRE( MexVec2( 0, 0 ).isZeroVector() );
    REQUIRE_FALSE( MexVec2( 1, 0 ).isZeroVector() );
    REQUIRE( MexVec2::zeroVector().isZeroVector() );
}

TEST_CASE( "MexVec2 dot product", "[mathex][vec2]" )
{
    MexVec2 a( 3, 4 );
    MexVec2 b( 1, 2 );

    REQUIRE( a.dotProduct( b ) == 11 );
    REQUIRE( a.dotProduct( a ) == a.squareModulus() );
}

TEST_CASE( "MexVec2 modulus and squareModulus", "[mathex][vec2]" )
{
    MexVec2 v( 3, 4 );

    REQUIRE( v.squareModulus() == 25 );
    REQUIRE( v.modulus() == Approx( 5.0 ) );
}

TEST_CASE( "MexVec2 normal is perpendicular", "[mathex][vec2]" )
{
    MexVec2 v( 3, 4 );
    MexVec2 n = v.normal();

    REQUIRE( n.x() == 4 );
    REQUIRE( n.y() == -3 );
    REQUIRE( v.dotProduct( n ) == 0 );
}

TEST_CASE( "MexVec2 += and -= are inverses", "[mathex][vec2]" )
{
    MexVec2 v( 1, 2 );
    MexVec2 delta( 5, -3 );

    v += delta;
    REQUIRE( v == MexVec2( 6, -1 ) );

    v -= delta;
    REQUIRE( v == MexVec2( 1, 2 ) );
}
