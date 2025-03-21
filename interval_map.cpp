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
        if ( ! ( keyBegin < keyEnd ) ) return;

        auto after = m_map.find( keyEnd );

        if ( after == m_map.end( ) )
        {
            //  If there's no end marker put one in unless its value would match our new interval.
            auto afterValue = ( * this )[ keyEnd ];
            if ( afterValue != val )
            {
                m_map.insert( { keyEnd, std::move( afterValue ) } );
            }
        }
        else
        {
            // If there is an end marker remove it if its value matches our new interval.
            if ( after -> second == val )
            {
                m_map.erase( after );
            }
        }
        
        auto first = m_map.lower_bound( keyBegin );
        auto last  = m_map.lower_bound( keyEnd );

        // Erase all the old intervals overwritten by this assign.
        if ( first != m_map.end( ) )
        {
            m_map.erase( first, last );
        }

        // Write our new interval iff the value differs from prior interval.
        if ( val != ( * this )[ keyBegin ] )
        {
            m_map.insert( { keyBegin, std::forward<V>( val ) } );
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

#include <iostream>
using namespace std;

void assign( interval_map<int, char> & map, int begin, int end, char value )
{
    cout << "assign( " << begin << ", " << end << ", " << value << " )" << endl;
    map.assign( begin, end, value );
}

template <typename Range> void print( const char * intro, char begin, const Range & range )
{
    cerr << intro << begin << " ";
    for ( auto & [ key, value ] : range ) cerr << "( " << key << ", " << value << " ), ";
    cerr << endl;
}

template <typename RangeOne, typename RangeTwo> bool equalRange( const RangeOne & r1, const RangeTwo & r2 )
{
    for ( auto [_1,_2] = tuple{ r1.begin( ), r2.begin( ) }; _1 != r1.end( ) && _2 != r2.end( ); ++ _1, ++_2 )
        if ( * _1 != * _2 ) return false;
    return r1.size( ) == r2.size( );
}

bool expect(
        char   actual_begin, const map<int, char> &                    actual,
        char expected_begin, initializer_list<pair<const int, char>> expected )
{
    if ( actual_begin == expected_begin && equalRange( actual, expected ) )
        return true;
    print( "Expected: ", expected_begin, expected );
    print( "Actual:   ",   actual_begin, actual   );
    return false;
}

void IntervalMapTest()
{
    {
        interval_map<int, char> map( 'A' );

                                 expect( map.m_valBegin, map.m_map, 'A', { } ); 
        assign( map, 0,4, 'B' ); expect( map.m_valBegin, map.m_map, 'A', { { 0, 'B' }, { 4, 'A' } } ); 
        assign( map, 1,3, 'A' ); expect( map.m_valBegin, map.m_map, 'A', { { 0, 'B' }, { 1, 'A' }, { 3, 'B' }, { 4, 'A' } } ); 
        assign( map, 3,4, 'C' ); expect( map.m_valBegin, map.m_map, 'A', { { 0, 'B' }, { 1, 'A' }, { 3, 'C' }, { 4, 'A' } } ); 
        assign( map, 0,2, 'A' ); expect( map.m_valBegin, map.m_map, 'A', { { 3, 'C' }, { 4, 'A' } } ); 
        assign( map, 0,4, 'C' ); expect( map.m_valBegin, map.m_map, 'A', { { 0, 'C' }, { 4, 'A' } } ); 
    }

    {
        interval_map<int, char> map( 'A' );

        assign( map, 0,2, 'B' );
        assign( map, 2,4, 'C' );
        assign( map, 4,6, 'D' ); expect( map.m_valBegin, map.m_map, 'A', { { 0, 'B' }, { 2, 'C' }, { 4, 'D' }, { 6, 'A' } } );

        assign( map, 1,5, 'A' ); expect( map.m_valBegin, map.m_map, 'A', { { 0, 'B' }, { 1, 'A' }, { 5, 'D' }, { 6, 'A' } } );

        assign( map, 0,3, 'D' ); expect( map.m_valBegin, map.m_map, 'A', { { 0, 'D' }, { 3, 'A' }, { 5, 'D' }, { 6, 'A' } } );

        assign( map,-1,6, 'A' ); expect( map.m_valBegin, map.m_map, 'A', { } );
    }

}

int main( )
{
    IntervalMapTest( );
}
