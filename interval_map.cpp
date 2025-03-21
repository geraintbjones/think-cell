#include <map>

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <array>


namespace test {

unsigned bounded_rand(unsigned range)
{
    for (unsigned x, r;;)
        if (x = std::rand(), r = x % range, x - r <= -range)
            return r;
}

//
// A dummy map wrapper that throws sometimes.
//
template<typename K, typename V> struct map {

    using base_map = std::map<K, V>;
    using iterator = base_map::iterator;
    using const_iterator = base_map::const_iterator;

    iterator begin( ) { return m_map.begin( ); }
    const_iterator begin( ) const { return m_map.begin( ); }

    iterator end( ) { return m_map.end( ); }
    const_iterator end( ) const { return m_map.end( ); }

    template<typename Key>
    iterator upper_bound( const Key & key ) {
        return m_map.upper_bound(key);
    }

    template<typename Key>
    const_iterator upper_bound( const Key & key ) const {
        return m_map.upper_bound(key);
    }

    template<typename V_fwd>
    iterator try_emplace( iterator hint, const K & key, V_fwd && val ) {
        return m_map.try_emplace(hint, key, std::forward<V>(val));
    }

    iterator insert_or_assign(iterator hint, const K & key, V && val) {
        if (bounded_rand(3) == 2) {
            throw std::bad_alloc( );
        };
        return m_map.insert_or_assign(hint, key, std::forward<V>(val)); }

    iterator erase(iterator pos) { return m_map.erase(pos); }
    iterator erase(iterator first, iterator last) { return m_map.erase(first, last); }

    base_map m_map;
};

} // namespace test

template<typename K, typename V>
class interval_map {
	friend void IntervalMapTest();
	V m_valBegin;
// 	test::map<K,V> m_map;
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
        using iterator = decltype(m_map)::iterator;

        if (!(keyBegin < keyEnd)) return;

        auto priorValue = [&](iterator element) -> V const & {
                return element == m_map.begin( ) ? m_valBegin : std::prev(element) -> second;
            };

        iterator afterEnd = m_map.upper_bound(keyEnd);
        V afterEndValue   = priorValue(afterEnd);
        bool didEndInsert = afterEnd == m_map.begin( ) || std::prev(afterEnd) -> first < keyEnd;
        iterator end      = m_map.try_emplace(afterEnd, keyEnd, std::move(afterEndValue));

        iterator begin;

        try {
            begin = m_map.insert_or_assign(end, keyBegin, std::forward<V>(val));
        } catch (...) {
            // Clean up work we've already done.
            if (didEndInsert) {
                m_map.erase(end);
            }
            throw;
        }

        // Accommodate canonicity.
        while (end != m_map.end( ) && end -> second == val) {
            ++ end;
        }

        if (!(begin -> second == priorValue(begin))) {
            ++ begin;
        }

        m_map.erase(begin, end);
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

using namespace std;

struct Key
{
    Key( int ) { }
    bool operator < ( Key const & ) const { return true; }
    bool operator > ( Key const & ) const = delete;
    bool operator == ( Key const & ) const = delete;
};

struct Value
{
    Value( char ) { }
    bool operator == ( Value const & ) const { return true; }
    bool operator != ( Value const & ) const = delete;
};

template <size_t size, typename R1, typename R2>
bool eq( const R1 & r1, const R2 & r2 )
{
    for ( int key = 0; key != size; ++ key )
        if ( r1[ key ] != r2[ key ] )
            return false;
    return true;
}

template <typename K, typename V, template<typename, typename> class map>
bool is_canonical(char m_valBegin, const map<K,V> & m_map )
{
    char last = m_valBegin;
    for ( auto const & element : m_map ) {
        if ( element.second == last ) return false;
        last = element.second;
    }
    return true;
}

template <typename T>
T id( T value ) { return value; }

template<typename intervals_type>
void intervals(const intervals_type & m_map) {
    cout << "Intervals:{ ";
//     for ( auto & [ key, value ] : map.m_map ) cout << "{ " << key << ", " << value << " } ";
    for ( auto element = m_map.begin(); element != m_map.end(); ++ element)
        cout << "{ " << element -> first << ", " << element -> second << " } ";
    cout << "}" << endl;

}

template<size_t size>
void ruler( ) {
    cout << "            0";
    for ( size_t key = 1; key != size; ++ key ) cout << "  " << ( key % 10 );
    cout << " }" << endl;
}

template<size_t size, typename map_type>
void actual(const map_type & map) {
    cout << "Actual    { " << map[ 0 ];
    for ( size_t key = 1; key != size; ++ key ) cout << ", " << map[ key ];
    cout << " }" << endl;
}

template<size_t size, typename ref_type>
void expected(const ref_type & ref) {
    cout << "Expected: { " << ref[ 0 ];
    for ( size_t key = 1; key != size; ++ key ) cout << ", " << ref[ key ];
    cout << " }" << endl << endl;
}

template<size_t size, char valBegin>
struct Ref {
    array<char, size> m_ref;

    Ref() { for ( auto & c : m_ref ) c = 'A'; }

    void assign(size_t begin, size_t end, char val) {
        for ( auto key = begin; key != end ; ++ key ) m_ref[ key ] = val;
    }
};

static int how_many = 10;

void IntervalMapTest()
{
    {
        // Instantiate with archetypes, no functional testing.
        interval_map<Key, Value> map( 'A' );
        map.assign( Key( 0 ), Key( 0 ), Value( 'B' ) );
    }

    constexpr size_t size = 20;
//     int how_many = 100000;

    interval_map<int, char> map('A');
    Ref<size, 'A'> ref;

    srand( time( 0 ) );

    while ( how_many )
    {
        int keyBegin = test::bounded_rand( size - 1 );
        int keyEnd   = test::bounded_rand( size - 1 );
        char value   = 'A' + test::bounded_rand( 6 ); // Values A - F, quite arbitrary.

        if ( ! ( keyBegin < keyEnd ) ) continue;

        cout << how_many -- << ":    assign( " << keyBegin << ", " << keyEnd << ", " << value << " )" << endl;

        try {
            map.assign( keyBegin, keyEnd, value );
            ref.assign( keyBegin, keyEnd, value );
        } catch (...) {
            cout << "Caught exception - check strong guarantee." << endl;
        }

        intervals(map.m_map);
        ruler<size>();
        actual<size>(map);
        expected<size>(ref.m_ref);

        if ( ! eq<size>( map, ref.m_ref ) || ! is_canonical( map.m_valBegin, map.m_map ) ) break;
    }
}

int main(int argc, char * argv[])
{
    if (argc == 2) {
        how_many = std::atoi(argv[1]);
        if (how_many) {
            IntervalMapTest( );
            return 0;
        }
    }
    cout << "Usage: " << argv[0] << "<number of randon tests>" << endl;
    return 1;
}
