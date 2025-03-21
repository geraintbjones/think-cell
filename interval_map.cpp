#include <map>

template<typename K, typename V>
class interval_map {
	friend void IntervalMapTest();
	V m_valBegin;
	std::map<K,V> m_map;
public:
	// constructor associates whole range of K with val
	template<typename V_forward>
	interval_map(V_forward&& val)
	: m_valBegin(std::forward<V_forward>(val))
	{}

	// Assign value val to interval [keyBegin, keyEnd).
	// Overwrite previous values in this interval.
	// Conforming to the C++ Standard Library conventions, the interval
	// includes keyBegin, but excludes keyEnd.
	// If !( keyBegin < keyEnd ), this designates an empty interval,
	// and assign must do nothing.
	template<typename V_forward>
	void assign( K const& keyBegin, K const& keyEnd, V_forward&& val )
		requires (std::is_same<std::remove_cvref_t<V_forward>, V>::value)
	{
        if (!(keyBegin < keyEnd)) return;

        auto after = m_map.find(keyEnd);

        if (after == m_map.end()) {
            //  If there's no end marker put one in unless its value would match our new interval.
            V const & afterValue = (* this)[keyEnd];
            if (!(afterValue == val)) {
                m_map.insert({keyEnd, afterValue});
            }
        } else {
            // If there is an end marker remove it if its value matches our new interval.
            if (after -> second == val) {
                m_map.erase(after);
            }
        }
        
        auto first = m_map.lower_bound(keyBegin);
        auto last  = m_map.lower_bound(keyEnd);

        // Erase all the old intervals overwritten by this assign.
        if (first != m_map.end()) {
            m_map.erase(first, last);
        }

        // Write our new interval iff the value differs from prior interval.
        if (!(val == (* this)[keyBegin])) {
            m_map.insert({keyBegin, std::forward<V>(val)});
        }
	}

	// look-up of the value associated with key
	V const& operator[]( K const& key ) const {
		auto it=m_map.upper_bound(key);
		if(it==m_map.begin()) {
			return m_valBegin;
		} else {
			return (--it)->second;
		}
	}
};

// Many solutions we receive are incorrect. Consider using a randomized test
// to discover the cases that your implementation does not handle correctly.
// We recommend to implement a test function that tests the functionality of
// the interval_map, for example using a map of int intervals to char.

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <array>

using namespace std;

struct Key
{
    Key( ) { }
    bool operator < ( Key const & ) const { return true; }
    bool operator > ( Key const & ) const = delete;
    bool operator == ( Key const & ) const = delete;
};

struct Value
{
    Value( ) { }
    Value( char ) { }
    bool operator == ( Value const & ) const { return true; }
    bool operator != ( Value const & ) const = delete;
};

template <typename Range> void print( const char * intro, char begin, const Range & range )
{
    cerr << intro << begin << " ";
    for ( auto & [ key, value ] : range ) cerr << "( " << key << ", " << value << " ), ";
    cerr << endl;
}

unsigned bounded_rand(unsigned range)
{
    for (unsigned x, r;;)
        if (x = rand(), r = x % range, x - r <= -range)
            return r;
}

template <typename R1, typename R2> bool eq20( const R1 & r1, const R2 & r2 )
{
    for ( int key = 0; key != 20; ++ key )
        if ( r1[ key ] != r2[ key ] )
            return false;
    return true;
}

void IntervalMapTest()
{
    {
        interval_map<Key, Value> map( 'A' );
        Value v = map[ Key( ) ];
        map.assign( Key( ), Key( ), Value( ) );
    }

    array<char, 20> ref;
    size_t size = ref.size( );
    interval_map<int, char> map('A');
    for ( auto & c : ref ) c = 'A';
    srand( time( 0 ) );

    while ( true )
    {
        int keyBegin = bounded_rand( size - 1 );
        int keyEnd   = bounded_rand( size - 1 );
        if ( ! ( keyBegin < keyEnd ) ) continue;
        char value = 'A' + bounded_rand( 6 );

        map.assign( keyBegin, keyEnd, value );
        for ( auto key = keyBegin; key != keyEnd ; ++ key ) ref[ key ] = value;

        cout << "assign( " << keyBegin << ", " << keyEnd << ", " << value << " )" << endl;

        cout << "{ ";
        for ( auto & [ key, value ] : map.m_map ) cout << "{ " << key << ", " << value << " } ";
        cout << "}" << endl;

        cout << "Actual    { " << map[ 0 ];
        for ( auto key = 1; key != size; ++ key ) cout << ", " << map[ key ];
        cout << " }" << endl;

        cout << "Expected: { " << ref[ 0 ];
        for ( auto key = 1; key != size; ++ key ) cout << ", " << ref[ key ];
        cout << " }" << endl;

        if ( eq20( map, ref ) ) continue;

        break;
    }
}

int main( )
{
    IntervalMapTest( );
}
