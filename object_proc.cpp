#ifndef log_type
#define log_type

#include "random_utilities.cpp"

enum type_t
{
    UNKNOWN,
    SPELL,
    AURA,
    SWING
};

class log_time_t
{
    std::string raw;
    double relative_time;
    double sequence;

    public:
    log_time_t( std::string str ) : raw( str )
    {
        constexpr int HRS = 3600;
        constexpr int MIN = 60;
        constexpr char hour_seperator = ':';
        constexpr int placements = 6;

        const size_t index = str.find( hour_seperator );

        double time = ( HRS * std::stoi( str.substr( 0, index ) ) );
        time += ( MIN * std::stoi( str.substr( index+1, index ) ) );
        time += std::stod( str.substr( (index+1)*2, placements ) );

        static const double log_start = time;

        if ( time > log_start )
            relative_time = time - log_start;
        else
            relative_time = time + log_start;

        static unsigned sequence_tracker = 0;
        sequence = ++sequence_tracker;
    }

    void shift_sequence( const double& p = 1.5 ) { this->sequence += p; }
    void shift_time( const double& p = 0.005 ) { this->relative_time += p; }
    void set_sequence( const double& p ) { this->sequence = p + this->sequence; }

    const std::string& get_raw_time() const { return this->raw; }
    const double& get_time() const { return this->relative_time; }
    const double& get_sequence() const { return this->sequence; }
};

class states_t
{
    // pretty much useless but i like the index being present
    static std::unordered_map<std::string, int> state_map;

    std::string str_state;
    int index_state;

    void construct_container( const std::string& str )
    {
        auto ind = state_map.find( str );
        if ( ind != state_map.end() )
            index_state = ind->second;
        else
            state_map.insert( { str, index_state = state_map.size() } );
        return;
    }

    public:
    states_t( const std::string& str ) : str_state( str )
    { construct_container( str ); }

    const bool operator==( const std::string& str ) const { return str_state == str; }

    const int& get_index_state() const { return this->index_state; }
    const std::string& get_str_state() const { return this->str_state; }
    static const std::unordered_map<std::string, int>& get_state_map() { return state_map; }
};
std::unordered_map<std::string, int> states_t::state_map;

class effect_t : public states_t
{
    type_t type; // Generic Type (Spell/Aura/Swing).
    std::string spell;
    std::string user;
    unsigned id;
    bool crit;

    bool set_crit( const std::string& str ) { return ( str == "1" ); }

    unsigned set_id( const std::string& str )
    {
        char* end;
        return *end ? 0 : id = std::strtol( str.c_str(), &end, 10 );
    }

    type_t set_type()
    {
        std::string_view s = this->get_str_state();
        if ( s.substr( s.find('_') + 1, std::string::npos ).substr(0, s.find('_') - 1 ) == "AURA" )
            return type_t::AURA;

        s = s.substr( 0 , s.find('_') );

        if      ( s == "SPELL" )  return type_t::SPELL;
        else if ( s == "SWING" )  return type_t::SWING;
        return type_t::UNKNOWN;
    }

    public:
    effect_t( const std::array<std::string, utils::log_index::CRIT+1>& fields ) :
        states_t( fields[utils::STATE] ),
        type( set_type() ),
        spell( utils::between_char( fields[utils::NAME] ) ),
        user( utils::between_char( fields[utils::PLAYER_USER], '-' ) ),
        id( set_id( fields[utils::ID] ) ),
        crit( set_crit( fields[utils::CRIT] ) )
    {}

    const effect_t& s() const { return *this; }
    const type_t& get_type() const { return this->type; }
    const std::string& get_spell() const { return this->spell; }
    const std::string& get_user() const { return this->user; }
    const unsigned& get_id() const { return this->id; }
    const bool& is_crit() const { return this->crit; }
};

struct log_object_t : public log_time_t, public effect_t
{
    bool dicey;
    bool target;
    unsigned chain_count;

    log_object_t( const std::string& s ) : log_object_t( utils::populate_index( s ) )
    {}

    log_object_t( const std::array<std::string, utils::log_index::CRIT+1>& data ) :
    log_time_t( data[ utils::log_index::TIME ] ),
    effect_t( data )
    {}

    log_object_t() = default;

    const effect_t& o() const { return *this; }
    const log_time_t& t() const { return *this; };

    std::string str( const bool& time_switch = false )
    {
        std::string output;
        if ( time_switch )
            output = t().get_sequence();
        else
            output = utils::format_zeros( t().get_time() );

        output += " ("+ ( t().get_raw_time() ) + ")";
        output += " " + utils::format_alignment( utils::format_zeros( t().get_sequence(), 6 ), 5 );
        output += " | " +utils::format_alignment( o().get_user() + " - ", 5 );
        output += utils::format_alignment( o().get_spell() + " " + ( o().is_crit() ? "(C)" : ""), 21 );
        output += " | " + utils::format_alignment( o().get_str_state(), 23 );
        output += ( " [" + std::to_string( o().get_index_state() ) + ']' );

        return output;
    }

    void set_dicey() { this->dicey = true; }
    const bool& is_dicey() const { return this->dicey; }

    void set_target( const bool& t ) { this->target = t; }
    const bool& is_target() const { return this->target; }

    void set_count( const unsigned& c = 1 ) { this->chain_count = c; }
    void increment_count( const unsigned& c = 1 ) { this->chain_count += c; }
    const unsigned& get_count() const { return this->chain_count; }
};
#endif