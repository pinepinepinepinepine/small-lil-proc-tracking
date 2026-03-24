#include "log_utils.cpp"

void parse_log( user_options_t& opts, std::ifstream& input, std::ofstream& output )
{
    // Pull from input file (raw log)
    std::string line;
    std::vector<log_object_t> log;
    while ( std::getline( input, line ) )
        log.emplace_back( line );

    // Filter whatever garbage
    std::vector<log_object_t> filtered_log = opts.filtered_output( log );

    // Counting
    // Could skip an iteration by moving log_utilities' counting into filtered_output but whatever! Future problem. Maybe.
    std::vector<std::unique_ptr<log_utilities_t>> count_log;
    for ( log_object_t& l : filtered_log )
        count_log.emplace_back( std::make_unique<log_utilities_t>( &l, l.o().get_spell() == opts.b()->get_buff_str() ) );

    // Output
    for ( std::unique_ptr<log_utilities_t>& p : count_log )
        output << p->output_log_object();
    log_utilities_t::log_s().make_summary( output );
}

int main()
{
    user_options_t opts;
    auto start = std::chrono::steady_clock::now();
    std::cout << "Starting Everything\n";

    // ================================================================== //

    // Change this to "log_input.txt" for Arcane's dump, then change the opts below (Freezing -> Arcane Salvo; Frost -> Arcane Splinter).
    std::ifstream input_file("frost_input.txt");
    std::ofstream output_file("log_output.txt");

    // Gets dicey with isolating procs, all this does is grab log -> filter -> count -> bye.
    // If, for example, we're targetting Freezing from Infused while casting Frostbolt, we can't filter out Freezing from other sources (Frostbolt).
    // Within this program, the fix is to assosciate a target with a trigger (to then isolate targets/buffs),
    // but alternatively... just don't mix stuff up in game.
    // Anyways, I'll fix if it's relevant later, or if I'm bored; for now, it's good enough, so yeah.

    opts.set_player( "Theepine" );

    opts.b()->set_buff( "Freezing" );
    opts.b()->set_exclude_buff_states_str( "SPELL_AURA_REMOVED" );
    opts.b()->set_exclude_buff_states_str( "SPELL_AURA_REMOVED_DOSE" );
    opts.b()->set_exclude_buff_states_str( "SPELL_AURA_REFRESH" );

    opts.s()->set_spell( "Frost Splinter" );

    parse_log( opts, input_file, output_file );

    // ================================================================== //

    std::cout << "\nDone Everything: ";
    std::cout << std::chrono::duration<double, std::milli>( std::chrono::steady_clock::now() - start ).count() << "ms\n";;
    return 0;
}