#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <assert.h>
#include <fixed_eytzinger_map.h>

static void test_basic_int_int()
{
    std::vector< std::pair<int, int> > d;
    int n = 30;
    for( int i = 0; i < n; ++i )
        d.emplace_back( i, i );
    fixed_eytzinger_map<int, int> e( std::begin(d), std::end(d) );

    for( int i = 0; i < n; ++i )
        assert( e.lower_bound(i)->first == i );

    for( int i = 0; i < n-1; ++i )
        assert( e.upper_bound(i)->first == i+1 );
    assert( e.upper_bound(n) == e.end() );

    for( int i = 0; i < n; ++i ) {
        auto t = e.equal_range(i);
        assert( t.first->first == i );
        assert( std::distance(t.first, t.second) == 1 );
    }
    
    for( int i = 0; i < n; ++i ) {
        assert( e.find(i) != e.end() && e.find(i)->first == i );
        assert( e.at(i) == i );
        assert( e[i] == i );
    }
    for( int i = n; i < 2*n; ++i )
        assert( e.find(i) == e.end() );
}

static void test_basic_int_string()
{
    std::vector< std::pair<int, std::string> > d;
    int n = 30;
    for( int i = 0; i < n; ++i )
        d.emplace_back( i, std::to_string(i) );
    
    fixed_eytzinger_map<int, std::string> e( std::begin(d), std::end(d) );

    for( int i = 0; i < n; ++i )
        assert( e.lower_bound(i)->first == i );

    for( int i = 0; i < n; ++i ) {
        assert( e.find(i) != e.end() && e.find(i)->first == i );
        assert( e.at(i) == std::to_string(i) );
        assert( e[i] == std::to_string(i) );
    }
    for( int i = n; i < 2*n; ++i )
        assert( e.find(i) == e.end() );
}

static void test_empty()
{
    fixed_eytzinger_map<int, int> m;
    assert( m.size() == 0 );
    assert( m.empty() );
    assert( m.count(42) == 0 );
    assert( begin(m) == end(m) );
}

static void test_iteration()
{
    std::vector< std::pair<int, int> > d;
    int n = 30;
    for( int i = 0; i < n; ++i )
        d.emplace_back( i, i );
    
    fixed_eytzinger_map<int, int> e( begin(d), end(d) );
    assert(
        std::accumulate(begin(e), end(e), 0, [](auto v1, const auto &v2){
            return v1 + v2.second;
        }) ==
        std::accumulate(begin(d), end(d), 0, [](auto v1, const auto &v2){
            return v1 + v2.second;
        })
        );
}

static void test_copy_and_move_semantics()
{
    fixed_eytzinger_map<int, int> a( {{0,0}, {1,0}} );
    assert( a.size() == 2 );
    fixed_eytzinger_map<int, int> b( move(a) );
    assert( a.size() == 0 );
    assert( b.size() == 2 );
    
    fixed_eytzinger_map<int, int> c( b );
    assert( c.size() == 2 );
    assert( b.size() == 2 );
    
    fixed_eytzinger_map<int, int> d;
    d = c;
    assert( d.size() == 2 );
    assert( d == c );
    
    fixed_eytzinger_map<int, int> e;
    e = move(d);
    assert( d.size() == 0 );
    assert( e.size() == 2 );
    
    d = {{0,0}, {1,0}, {2,0}};
    assert( d.size() == 3 );
    d.assign( begin(e), end(e) );
    assert( d.size() == 2 );
}

static void test_move_semantics()
{
    struct A
    {
        A(int){};
        A(const A&){ assert(0); }
        A(A&&) noexcept {}
        A& operator=(const A&){ assert(0);}
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
    assert( a.size() == 4 );
    const A &z = a[0];
    (void)z;
    
    fixed_eytzinger_map<int, A> b( move(a) );
    assert( a.size() == 0 );
    assert( b.size() == 4 );
    
    std::swap( a, b );
    assert( a.size() == 4 );
    assert( b.size() == 0 );
    
    a.clear();
    assert( a.size() == 0 );
    
    std::vector< std::pair<std::string, A> > e;
    e.emplace_back( std::make_pair("Abra", A(0)) );
    fixed_eytzinger_map<std::string, A> f(std::make_move_iterator(e.begin()),
                                          std::make_move_iterator(e.end())
                                          );
    const A &g = f["Abra"];
    (void)g;
}

static void test_heteregenous_lookup()
{
    fixed_eytzinger_map<std::string, std::string, std::less<>>
        a( {{"a", "a"}, {"b", "b"}, {"c", "c"}} );
 
    assert( a.lower_bound("a")->second == "a" );
    assert( a.upper_bound("a")->second == "b" );
    assert( a.find("a")->second == "a" );
    assert( a.count("a") == 1 );
    assert( a.count("z") == 0 );
    assert( a.at("c") == "c" );
    assert( a["b"] == "b" );
}

int main()
{
    test_basic_int_int();
    test_basic_int_string();
    test_empty();
    test_iteration();
    test_copy_and_move_semantics();
    test_move_semantics();
    test_heteregenous_lookup();
    return 0;
}
