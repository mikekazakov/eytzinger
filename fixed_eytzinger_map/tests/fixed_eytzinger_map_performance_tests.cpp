#include <sys/resource.h>
#include <sys/proc_info.h>
#include <libproc.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <memory>
#include <map>
#include <thread>
#include <unordered_map>
#include <assert.h>
#include <chrono>
#include <array>
#include <random>
#include <boost/container/flat_map.hpp>
#include <fixed_eytzinger_map.h>

using namespace std;
using namespace std::chrono;

template<typename F>
nanoseconds measure_time(F f)
{
    static const auto num_trials = 20;
    static const auto min_time_per_trial = milliseconds{200};
    array<nanoseconds, num_trials> trials;
    volatile static decltype(f()) res;
    
    for( auto &trial: trials) {
        int runs = 0;
        const auto t1 = chrono::high_resolution_clock::now();
        auto t2 = t1;
        for(;
            t2 - t1 < min_time_per_trial;
            ++runs, t2 = chrono::high_resolution_clock::now() )
            res = f();
        
        trial = duration_cast<nanoseconds>(t2 - t1) / runs;
    }
    
    sort( trials.begin(), trials.end() );
    auto avg = accumulate( trials.begin()+2, trials.end()-2, nanoseconds{0} ) / (trials.size()-4);
    return duration_cast<nanoseconds>(avg);
}

struct rand_seq
{
    rand_seq(int n):
        dist(0, n-1),
        gen(34862)
    {}
    int operator()(){ return dist(gen); }
    
private:
    uniform_int_distribution<int> dist;
    mt19937                       gen;
};

void fill_test_data( vector< pair<int, int> > &d, int n )
{
    for( int i = 0; i < n; ++i )
        d.emplace_back( i, i );
}

template <typename C>
auto spawn(int _n)
{
    vector< pair<int, int> > d;
    fill_test_data(d, _n);
    return C{ begin(d), end(d) };
}

template <typename C>
auto lookup(const C&_c, int _n)
{
    rand_seq rnd(_n);

    uint64_t sum = 0;
    while( _n-- )
        sum += _c.count( rnd() );
    return sum;
}

void test_lookup()
{
    cout << "lookup times, us per element" << endl;
    cout << "n" <<
        ";" << "std::map<int,int>" <<
        ";" << "std::unordered_map<int,int>" <<
        ";" << "boost::flat_map<int,int>" <<
        ";" << "fixed_eytzinger_map<int,int>" << endl;
    
    const int n1 = 1000, n2 = 10000000, d = 200;
    const double dp = 1.2;
    
    for( int n = n1, dn = d; n <= n2; n += dn, dn *= dp ) {
        cout << n << ";";
        {
            auto m1 = spawn<map<int, int>>(n);
            auto t = measure_time( [&]{ return lookup(m1, n); });
            cout << double(t.count()) / n / 1E3  << ";";
        }
        
        {
            auto m2 = spawn<unordered_map<int, int>>(n);
            auto t = measure_time( [&]{ return lookup(m2, n); });
            cout << double(t.count()) / n / 1E3  << ";";
        }
        
        {
            auto m3 = spawn<boost::container::flat_map<int, int>>(n);
            auto t = measure_time( [&]{ return lookup(m3, n); });
            cout << double(t.count()) / n / 1E3  << ";";
        }
        
        {
            auto m4 = spawn<fixed_eytzinger_map<int, int>>(n);
            auto t = measure_time( [&]{ return lookup(m4, n); });
            cout << double(t.count()) / n / 1E3  << endl;
        }
    }
}

template <typename C>
auto lookup_and_fetch(const C&_c, int _n)
{
    rand_seq rnd(_n);

    uint64_t sum = 0;
    while( _n-- )
        sum += _c.at( rnd() );
    return sum;
}

void test_lookup_and_fetch()
{
    cout << "lookup and fetch times, us per element" << endl;
    cout << "n" <<
        ";" << "std::map<int,int>" <<
        ";" << "std::unordered_map<int,int>" <<
        ";" << "boost::flat_map<int,int>" <<
        ";" << "fixed_eytzinger_map<int,int>" << endl;
    
    const int n1 = 1000, n2 = 10000000, d = 200;
    const double dp = 1.2;
    
    for( int n = n1, dn = d; n <= n2; n += dn, dn *= dp ) {
        cout << n << ";";
        {
            auto m1 = spawn<map<int, int>>(n);
            auto t = measure_time( [&]{ return lookup_and_fetch(m1, n); });
            cout << double(t.count()) / n / 1E3  << ";";
        }
        
        {
            auto m2 = spawn<unordered_map<int, int>>(n);
            auto t = measure_time( [&]{ return lookup_and_fetch(m2, n); });
            cout << double(t.count()) / n / 1E3  << ";";
        }
        
        {
            auto m3 = spawn<boost::container::flat_map<int, int>>(n);
            auto t = measure_time( [&]{ return lookup_and_fetch(m3, n); });
            cout << double(t.count()) / n / 1E3  << ";";
        }
        
        {
            auto m4 = spawn<fixed_eytzinger_map<int, int>>(n);
            auto t = measure_time( [&]{ return lookup_and_fetch(m4, n); });
            cout << double(t.count()) / n / 1E3  << endl;
        }
    }
}

vector< pair<int, int> > spawn_test_data(int _n)
{
    vector< pair<int, int> > d;
    fill_test_data(d, _n);
    return d;
}

void test_building()
{
    cout << "build&destroy times, us per element" << endl;
    cout << "n" <<
        ";" << "std::map<int,int>" <<
        ";" << "std::unordered_map<int,int>" <<
        ";" << "boost::flat_map<int,int>" <<
        ";" << "fixed_eytzinger_map<int,int>" << endl;
    
    const int n1 = 1000, n2 = 10000000, d = 200;
    const double dp = 1.2;
    
    for( int n = n1, dn = d; n <= n2; n += dn, dn *= dp ) {
        cout << n << ";";
        {
            auto d = spawn_test_data(n);
            auto t = measure_time( [&]{
                return map<int, int>{ begin(d), end(d) }.count(n);
            });
            cout << double(t.count()) / n / 1E3  << ";";
        }
        
        {
            auto d = spawn_test_data(n);
            auto t = measure_time( [&]{
                return unordered_map<int, int>{ begin(d), end(d) }.count(n);
            });
            cout << double(t.count()) / n / 1E3  << ";";
        }
        
        {
            auto d = spawn_test_data(n);
            auto t = measure_time( [&]{
                return boost::container::flat_map<int, int>{ begin(d), end(d) }.count(n);
            });
            cout << double(t.count()) / n / 1E3  << ";";
        }
        
        {
            auto d = spawn_test_data(n);
            auto t = measure_time( [&]{
                return fixed_eytzinger_map<int, int>{ begin(d), end(d) }.count(n);
            });
            cout << double(t.count()) / n / 1E3  << endl;
        }
    }
}

uint64_t mem_usage()
{
    rusage_info_current usage;
    proc_pid_rusage( getpid(), RUSAGE_INFO_CURRENT, (void**)&usage );
    return usage.ri_phys_footprint;
}

int main_mem_slave( int _t, int _n )
{
    const auto mem_init = mem_usage();
    auto mem_after = mem_init;
    
    if( _t == 0 ) {
        auto m1 = spawn<map<int, int>>(_n);
        mem_after = mem_usage() + m1.count(_n);
    }
    else if( _t == 1 ) {
        auto m2 = spawn<unordered_map<int, int>>(_n);
        mem_after = mem_usage() + m2.count(_n);
    }
    else if( _t == 2 ) {
        auto m3 = spawn<boost::container::flat_map<int, int>>(_n);
        mem_after = mem_usage() + m3.count(_n);
    }
    else if( _t == 3 ) {
        auto m4 = spawn<fixed_eytzinger_map<int, int>>(_n);
        mem_after = mem_usage() + m4.count(_n);
    }
    
    cout << to_string( mem_after - mem_init );
    
    return 0;
}

string exec(const string &_cmd)
{
    array<char, 128> buffer;
    string result;
    shared_ptr<FILE> pipe{ popen(_cmd.c_str(), "r"), pclose };
    if( !pipe )
        throw runtime_error("popen() failed!");
    
    while( !feof( pipe.get() ) )
        if( fgets( buffer.data(), 128, pipe.get() ) )
            result += buffer.data();
    
    return result;
}

long avg_exec_value(const string &_cmd)
{
    static const int num_trials = 20;
    array<long, num_trials> trials;
   
     for( auto &i: trials )
        i = atol( exec(_cmd).c_str() );
    
    sort( trials.begin(), trials.end() );
    return accumulate( trials.begin()+2, trials.end()-2, 0ul ) / (trials.size()-4);
}

void test_memory( const string &_bin_path )
{
    cout << "memory consumption by process, bytes per element" << endl;
    cout << "n" <<
        ";" << "std::map<int,int>" <<
        ";" << "std::unordered_map<int,int>" <<
        ";" << "boost::flat_map<int,int>" <<
        ";" << "fixed_eytzinger_map<int,int>" << endl;
    
    const int n1 = 1000, n2 = 10000000, d = 200;
    const double dp = 1.2;
    
    for( int n = n1, dn = d; n <= n2; n += dn, dn *= dp ) {
        cout << n << ";";
        cout << double(avg_exec_value(_bin_path + " 0 " + to_string(n))) / n << ";";
        cout << double(avg_exec_value(_bin_path + " 1 " + to_string(n))) / n << ";";
        cout << double(avg_exec_value(_bin_path + " 2 " + to_string(n))) / n << ";";
        cout << double(avg_exec_value(_bin_path + " 3 " + to_string(n))) / n << endl;
    }
}

int main(int argc, const char * argv[])
{
    if( argc == 3 )
        return main_mem_slave( atoi(argv[1]), atoi(argv[2]) );

    test_lookup();
    cout << endl;

    test_lookup_and_fetch();
    cout << endl;

    test_building();
    cout << endl;

    test_memory( argv[0] );
    cout << endl;

    return 0;
}
