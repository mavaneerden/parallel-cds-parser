/**
 * Author:
 *   Marco van Eerden
 * Description:
 *   Implementation of the Parser base class.
 */

#include "parser.hpp"

/**
 * @brief Constructs a parser using a grammar.
 *
 * @param g Grammar.
 */
Parser::Parser(Grammar g) : grammar(g), timer(Timer()) {}

/**
 * @brief Sets the appropriate varibles and starts the timer. Calls the
 * virtual loop() and print_data() functions.
 *
 * @param input_sequence Input sequence for the parser.
 *
 * @return Tuple with descriptors and EPNs outputted by the parser.
 */
std::tuple<descriptor_set_t, epn_set_t>
Parser::parse(std::vector<std::string> input_sequence)
{
    this->input = input_sequence;
    this->timer.start();

    auto result = loop();

    this->timer.stop();

    print_data();

    return result;
}