#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <vector>
#include <string>
#include <exception>
#include <algorithm>
#include <numeric>
#include <fixed_eytzinger_map.h>

TEST_CASE( "Works with int->int", "[fixed_eytzinger_map]" )
{
    std::vector< std::pair<int, int> > d;
    int n = 30;
    for( int i = 0; i < n; ++i )
        d.emplace_back( i, i );
    fixed_eytzinger_map<int, int> e{ std::begin(d), std::end(d) };
    
    SECTION( "size" ) {
        CHECK( e.size() == d.size() );
        CHECK( e.empty() == false );
    }
    
    SECTION( "lower_bound" ) {
        for( int i = 0; i < n; ++i ) {
            REQUIRE( e.lower_bound(i) != e.end() );
            CHECK( e.lower_bound(i)->first == i );
        }
    }
    
    SECTION( "upper_bound" ) {
        for( int i = 0; i < n-1; ++i )
            CHECK( e.upper_bound(i)->first == i+1 );
        CHECK( e.upper_bound(n) == e.end() );
    }
    
    SECTION( "equal_range" ) {
        for( int i = 0; i < n; ++i ) {
            auto t = e.equal_range(i);
            CHECK( t.first->first == i );
            CHECK( t.first->second == i );
            CHECK( std::distance(t.first, t.second) == 1 );
        }
    }
    
    SECTION( "find" ) {
        for( int i = 0; i < n; ++i ) {
            REQUIRE( e.find(i) != e.end() );
            CHECK( e.find(i)->first == i );
            CHECK( e.at(i) == i );
            CHECK( e[i] == i );
        }
        for( int i = n; i < 2*n; ++i )
            CHECK( e.find(i) == e.end() );
        for( int i = -2*n; i < 0; ++i )
            CHECK( e.find(i) == e.end() );
    }
}

TEST_CASE( "Works with int->string", "[fixed_eytzinger_map]" )
{
    std::vector< std::pair<int, std::string> > d;
    int n = 30;
    for( int i = 0; i < n; ++i )
        d.emplace_back( i, std::to_string(i) );
    
    fixed_eytzinger_map<int, std::string> e{ std::begin(d), std::end(d) };

    for( int i = 0; i < n; ++i ) {
        REQUIRE( e.lower_bound(i) != e.end() );
        CHECK( e.lower_bound(i)->first == i );
    }

    for( int i = 0; i < n; ++i ) {
        REQUIRE( e.find(i) != e.end() );
        CHECK( e.find(i)->first == i );
        CHECK( e.at(i) == std::to_string(i) );
        CHECK( e[i] == std::to_string(i) );
    }
    for( int i = n; i < 2*n; ++i )
        CHECK( e.find(i) == e.end() );
}


TEST_CASE( "Has a valid empty semantics", "[fixed_eytzinger_map]" )
{
    fixed_eytzinger_map<int, int> m;
    CHECK( m.size() == 0 );
    CHECK( m.empty() );
    CHECK( m.count(42) == 0 );
    CHECK( begin(m) == end(m) );
    
    m.assign( {{0, 0}, {1, 1}} );
    CHECK( m.size() == 2 );
    CHECK( !m.empty() );
    CHECK( m.count(42) == 0 );
    CHECK( begin(m) != end(m) );
    
    m = {};
    CHECK( m.size() == 0 );
    CHECK( m.empty() );
    CHECK( m.count(42) == 0 );
    CHECK( begin(m) == end(m) );
}

TEST_CASE( "Iterators cover whole data", "[fixed_eytzinger_map]" )
{
    std::vector< std::pair<int, int> > d;
    int n = 30;
    for( int i = 0; i < n; ++i )
        d.emplace_back( i, i );
    
    fixed_eytzinger_map<int, int> e( begin(d), end(d) );
    auto sum_map = std::accumulate(std::begin(e),
                                   std::end(e),
                                   0,
                                   [](int v1, const std::pair<int, int> &v2){
        return v1 + v2.second;
    });
    auto sum_source = std::accumulate(std::begin(d),
                                      std::end(d),
                                      0,
                                      [](int v1, const std::pair<int, int> &v2){
        return v1 + v2.second;
    });
    CHECK( sum_map == sum_source );
}

TEST_CASE( "Supports copy and move semantics", "[fixed_eytzinger_map]" )
{
    fixed_eytzinger_map<int, int> a( {{0,0}, {1,0}} );
    CHECK( a.size() == 2 );
    fixed_eytzinger_map<int, int> b( std::move(a) );
    CHECK( a.size() == 0 );
    CHECK( b.size() == 2 );
    
    fixed_eytzinger_map<int, int> c( b );
    CHECK( c.size() == 2 );
    CHECK( b.size() == 2 );
    
    fixed_eytzinger_map<int, int> d;
    d = c;
    CHECK( d.size() == 2 );
    CHECK( d == c );
    
    fixed_eytzinger_map<int, int> e;
    e = std::move(d);
    CHECK( d.size() == 0 );
    CHECK( e.size() == 2 );
    
    d = {{0,0}, {1,0}, {2,0}};
    CHECK( d.size() == 3 );
    d.assign( begin(e), end(e) );
    CHECK( d.size() == 2 );
}

TEST_CASE( "Supports move-only semantics", "[fixed_eytzinger_map]" )
{
    struct A
    {
        A(int){};
        A(const A&){ REQUIRE(false); }
        A(A&&) noexcept {}
        A& operator=(const A&){ REQUIRE(false); throw std::logic_error(""); }
        A& operator=(A&&) noexcept { return *this; }
    };

    std::vector< std::pair<int, A>  > d;
    d.emplace_back( std::make_pair(0, A(0)) );
    d.emplace_back( std::make_pair(1, A(1)) );
    d.emplace_back( std::make_pair(2, A(2)) );
    d.emplace_back( std::make_pair(3, A(3)) );

    fixed_eytzinger_map<int, A> a(  std::make_move_iterator(d.begin()),
                                    std::make_move_iterator(d.end())
                                );
    CHECK( a.size() == 4 );
    const A &z = a[0];
    (void)z;
    
    fixed_eytzinger_map<int, A> b( std::move(a) );
    CHECK( a.size() == 0 );
    CHECK( b.size() == 4 );
    
    std::swap( a, b );
    CHECK( a.size() == 4 );
    CHECK( b.size() == 0 );
    
    a.clear();
    CHECK( a.size() == 0 );
    
    std::vector< std::pair<std::string, A> > e;
    e.emplace_back( std::make_pair("Abra", A(0)) );
    fixed_eytzinger_map<std::string, A> f(std::make_move_iterator(e.begin()),
                                          std::make_move_iterator(e.end())
                                          );
    const A &g = f["Abra"];
    (void)g;
}

#if __cplusplus >= 201402L
TEST_CASE( "Supports heteregenous lookup", "[fixed_eytzinger_map]" )
{
    fixed_eytzinger_map<std::string, std::string, std::less<>>
        a( {{"a", "a"}, {"b", "b"}, {"c", "c"}} );
 
    CHECK( a.lower_bound("a")->second == "a" );
    CHECK( a.upper_bound("a")->second == "b" );
    CHECK( a.find("a")->second == "a" );
    CHECK( a.count("a") == 1 );
    CHECK( a.count("z") == 0 );
    CHECK( a.at("c") == "c" );
    CHECK( a["b"] == "b" );
}
#endif

TEST_CASE( "Removes duplicate keys", "[fixed_eytzinger_map]" )
{
    fixed_eytzinger_map<std::string, std::string> a(
        {{"a", "a"}, {"a", "a"}, {"b", "b"}, {"b", "b"}, {"c", "c"}, {"c", "c"}} );
    
    CHECK( a.size() == 3 );
    CHECK( a["a"] == "a" );
    CHECK( a["b"] == "b" );
    CHECK( a["c"] == "c" );
}

TEST_CASE( "Supports custom comparators for non-comparable types", "[fixed_eytzinger_map]" )
{
    struct Key {
        int v;
    };
    struct Compare {
        bool operator()(const Key& _1st, const Key& _2nd) const { return _1st.v < _2nd.v; };
    };
    
    std::vector< std::pair<Key, int> > d;
    d.emplace_back( Key{1}, 1 );
    d.emplace_back( Key{2}, 2 );
    d.emplace_back( Key{3}, 3 );
    d.emplace_back( Key{4}, 4 );
    
    auto map = fixed_eytzinger_map<Key, int, Compare>{ std::begin(d), std::end(d) };
    CHECK( map.size() == 4 );
    CHECK( map[Key{1}] == 1 );
    CHECK( map[Key{2}] == 2 );
    CHECK( map[Key{3}] == 3 );
    CHECK( map[Key{4}] == 4 );
}

#if __cplusplus >= 201402L
struct HC_Key {
    int v;
};
struct HC_Compare {
    struct is_transparent{};
    
    bool operator()(const HC_Key& _1st, const HC_Key& _2nd) const { return _1st.v < _2nd.v; };
    template <typename K2>
    bool operator()(const HC_Key& _1st, const K2& _2nd) const { return _1st.v < _2nd; };
    template <typename K1>
    bool operator()(const K1& _1st, const HC_Key& _2nd) const { return _1st < _2nd.v; };
};
TEST_CASE( "Supports heteregenous comparators for non-comparable types", "[fixed_eytzinger_map]" )
{
    std::vector< std::pair<HC_Key, int> > d;
    d.emplace_back( HC_Key{1}, 1 );
    d.emplace_back( HC_Key{2}, 2 );
    d.emplace_back( HC_Key{3}, 3 );
    d.emplace_back( HC_Key{4}, 4 );
    
    auto map = fixed_eytzinger_map<HC_Key, int, HC_Compare>{ std::begin(d), std::end(d) };
    CHECK( map.size() == 4 );
    CHECK( map[1] == 1 );
    CHECK( map[2.] == 2 );
    CHECK( map[3.f] == 3 );
    CHECK( map[4l] == 4 );
}
#endif
