#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <memory>
#include <array>
#include <unordered_map>


namespace utils
{
    enum log_index
    {
        TIME = 0,
        STATE = 1,
        PLAYER_USER = 3,
        TARGET = 7,
        ID = 10,
        NAME = 11,
        CRIT = 39
    };

    // I'm using a fixed array because it's good enough and a vector takes way, way longer (well, relative to whatever it's at right now)
    std::array<std::string, log_index::CRIT+1> populate_index( const std::string& str )
    {
        std::array<std::string, log_index::CRIT+1> fields {};

        size_t end = str.find("  ") + 1;
        size_t start = str.find(" ");

        fields[log_index::TIME] = str.substr( start + 1, str.find("-") - start - 1 );

        for ( int z = log_index::STATE; z < fields.size(); z++ )
        {
            start = end;
            end = str.find_first_of( ',' , end+1 );
            fields[z] = str.substr( start + 1, end - start - 1 );

            if ( end == std::string::npos )
                return fields;
        }
        return fields;
    }

    std::string format_zeros( const double& n, const size_t& placements = 3 )
    {
        std::string s = std::to_string( n );
        for ( int i = 0; i < placements + 1; i++ )
            if ( ( s.back() == '0' && i < placements ) || s.back() == '.' )
                s.pop_back();
            else
                return s;

        return s;
    }

    std::string format_alignment( const std::string& sv, const unsigned placements = 21 )
    {
        unsigned s_len = sv.length();
        if ( !placements || s_len >= placements )
            return static_cast<std::string>(sv);

        return static_cast<std::string>(sv).append( placements - s_len, ' ' );
    }

    std::string between_char( const std::string& str, const char end = '"', const char start = '"' )
    {
        const size_t a = str.find( start );
        if ( a == std::string::npos )
            return str;

        const size_t b = start == end ? str.rfind( end ) : str.find( end ) ;
        if ( b == std::string::npos )
            return str;

        return static_cast<std::string>( str ).substr( a+1, b - a - 1 );
    }
}