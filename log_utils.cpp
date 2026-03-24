#include "object_proc.cpp"

// Counting Stuff
struct log_stats_t
{
    unsigned total_casts = 0;
    unsigned total_successes = 0;
    std::vector<unsigned> total_casts_at;
    std::vector<unsigned> total_successes_at;

    void count_summary( const log_object_t* o )
    {
        if ( !o )
            return;

        if ( o->is_target() )
        {
            total_successes++;
            if ( o->get_count() >= total_successes_at.size() )
                total_successes_at.resize( o->get_count() + 1 );
            total_successes_at[o->get_count()]++;
        }
        else
        {
            total_casts++;
            if ( o->get_count() >= total_casts_at.size() )
                total_casts_at.resize( o->get_count() + 1 );
            total_casts_at[o->get_count()]++;
        }
    }

    void make_summary( std::ofstream& of ) const
    {
        std::vector<std::string> percentages;

        if ( !total_successes || !total_casts )
            return;

        of << "\n" << total_successes << "(Procs) / " << total_casts << "(Casts)" << " = " << total_successes / static_cast<double>(total_casts) * 100 <<  "%\n";
        for ( int i = 0; i < total_successes_at.size(); i++ )
        {
            if ( total_successes_at[i] && total_casts_at[i] )
            {
                percentages.push_back( std::to_string( total_casts_at[i] ? total_successes_at[i] / static_cast<double>( total_casts_at[i] ) : 0 ) );

                of << utils::format_alignment( std::to_string( i ) + ": " + std::to_string( total_successes_at[i] ) + " (/"+ std::to_string( total_casts_at[i] ) + ")", 17 );
                of << ( "= " + percentages.back() );
                of << ( i % 4 ? " | " : " |\n");
            }
            else
                percentages.push_back( "0" );
        }

        of << "\n[ ";
        for ( std::string& p : percentages )
            of << p << " ";
        of << "]";
    }
};

class log_utilities_t
{
    static log_stats_t stats;
    static unsigned chain_tracker;
    static log_object_t* snapshot_previous;

    log_object_t* original;

    public:
    log_utilities_t( log_object_t* lo, const bool& t )
    : original( lo )
    {
        original->set_target( t );

        if ( snapshot_previous && snapshot_previous->is_target() )
            chain_tracker = 0;
        if ( !snapshot_previous || !original->is_target() )
            chain_tracker++;
        original->set_count( chain_tracker );

        if ( snapshot_previous && snapshot_previous->is_target() && !original->is_target() )
        {
            const double o_t = original->t().get_time();
            const double s_t = snapshot_previous->t().get_time();

            // Some nearly arbitrary value, just needs to be high enough to accomodate dicey procs, but less than a prev cast.
            if ( o_t - s_t <= 0.005 )
            {
                original->set_dicey();
                snapshot_previous->set_dicey();

                // Not necessary to shift time. I prefer the visual indicator of it being 1ms below.
                original->shift_time( s_t - o_t - 0.001 );

                snapshot_previous->increment_count();
                original->set_count( snapshot_previous->get_count() );

                const log_object_t temp_hold = *snapshot_previous;
                *snapshot_previous = *original;
                *original = temp_hold;
            }
        }
        snapshot_previous = original;
    }

    log_utilities_t() = default;

    // Whatever
    static bool compare_sequence_time( const log_object_t& c_a, const log_object_t& c_b )
    { return c_a.t().get_sequence() < c_b.t().get_sequence(); }

    static bool compare_relative_time( const log_object_t& c_a, const log_object_t& c_b )
    { return c_a.t().get_time() < c_b.t().get_time(); }

    std::string output_log_object()
    {
        // You COULD move this to count_summary in the constructor, but if you pass snapshot_previous, it'll exclude the last line in a log.
        // This is because we can't be certain at construction time whether this instance is 'dicey' as it's dependent on the previous object.
        // Alternatively, we could look at whether or not the 'PREVIOUS of ORIGINAL's' delay is close enough to one another,
        // but it's dicey (hehe) as it's dependant on the delay from spell -> proc, which is inconsistent.
        // You COULD just manually call count_summary for the last object when iterating (but it's ugly),
        // and you COULD rewrite the logic to manually account for diceys at construction time.
        // This is perfectly fine... for now.
        stats.count_summary( original );

        bool target = original->is_target();
        std::string count_format = ( target ? "{" : "(" ) + std::to_string( original->get_count() ) + ( target ? "}" : ")" ) + ( original->is_dicey() ? "*" : "" );
        return utils::format_alignment( count_format, 5 ) + " | " + original->str() + ( target ? "\n\n" : "\n" );
    }

    log_object_t* log() const { return this->original; }
    static log_stats_t& log_s() { return stats; }
};
log_stats_t log_utilities_t::stats;
log_object_t* log_utilities_t::snapshot_previous = nullptr;
unsigned log_utilities_t::chain_tracker = 0;



// Filtering Stuff
struct spell_opt_t
{
    std::vector<std::string> include_spells;
    std::vector<std::string> exclude_states_str;

    const bool filter_spell_str( const log_object_t& l ) const
    {
        if ( !include_spells.size() )
            return true;

        for ( const std::string& s : include_spells )
        {
            if ( l.get_spell() == s )
                return true;
        }
        return false;
    }

    const bool filter_spell( const log_object_t& l ) const
    {
        if ( !filter_spell_str( l ) )
            return false;

        if ( !exclude_states_str.size() )
            return true;

        for ( const std::string& s : exclude_states_str )
        {
            if ( l.get_str_state() == s )
                return false;
        }
        return true;
    }

    void set_spell( const std::string& s ) { include_spells.push_back( s ); }

    void set_exclude_spell_states_str( const std::string& s )
    { exclude_states_str.push_back( s ); }
};

struct buff_opt_t
{
    std::string buff;
    std::vector<std::string> exclude_states_str;

    const bool filter_buff( const log_object_t& l ) const
    {
        if ( buff.empty() )
            return true;

        if ( l.get_spell() != this->buff )
            return false;

        if ( !exclude_states_str.size() )
            return true;

        for ( const std::string& s : exclude_states_str )
        {
            if ( l.get_str_state() == s )
                return false;
        }
        return true;
    }

    void set_buff( const std::string& b ) { this->buff = b; }

    void set_exclude_buff_states_str( const std::string& b )
    { exclude_states_str.push_back( b ); }

    const std::string& get_buff_str() const { return this->buff; }
};

struct user_options_t
{
    std::string player_name;
    spell_opt_t spell;
    buff_opt_t buff;

    const std::vector<log_object_t> filtered_output( const std::vector<log_object_t>& list )
    {
        std::vector<log_object_t> outputVector;
        for ( const log_object_t& l : list )
        {
            if ( l.get_user() == this->player_name || this->player_name.empty() )
            {
                if ( this->spell.filter_spell( l ) || this->buff.filter_buff( l ) )
                    outputVector.push_back( l );
            }
        }
        return outputVector;
    }

    void set_player( const std::string& user ) { this->player_name = user; }

    buff_opt_t* b() { return &buff; }
    spell_opt_t* s() { return &spell; }
};