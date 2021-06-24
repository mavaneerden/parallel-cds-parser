/**
 * Author:
 *   Marco van Eerden
 * Description:
 *   Entry point of the program. There are two command line arguments:
 *   The first argument is the file that contains the grammar. Each line must
 *   be formatted in the following way: <lhs> <rhs1> <rhs2> ... <rhsn>.
 *   The second argument is either a file that contains the input string or the
 *   input string itself. The symbols must be separated by spaces.
 */

#include <iostream>
#include "utilities/argparse.hpp"
#include "utilities/print.hpp"
#include "utilities/checks.hpp"
#include "components/grammar.hpp"
#include "parsers/sequential/sequential_parser.hpp"
#include "parsers/parallel_pool/parallel_pool.hpp"
#include "parsers/parallel_tree/parallel_tree.hpp"

/**
 * @brief Validates the correctness of the results.
 *
 * @param result Tuple containing the results.
 * @param input Input sequence.
 * @param grammar Input grammar.
 */
void validate_result(std::tuple<descriptor_set_t, epn_set_t> result, std::vector<std::string> input, Grammar grammar)
{
    bool success = check_correctness(std::get<0>(result), std::get<1>(result), grammar, input);

    if (success)
    {
        std::cout << "Output is correct." << std::endl;
    }
}

/**
 * @brief Print the EPNs and descriptors for the parser.
 *
 * @param title Title of the results.
 * @param result Tuple containing the results.
 */
void print_result(std::string title, std::tuple<descriptor_set_t, epn_set_t> result)
{
    std::cout << title << std::endl;
    std::cout << "EPNs:" << std::endl;
    print_epns(std::get<1>(result));
    std::cout << "Descriptors:" << std::endl;
    print_descriptors(std::get<0>(result));
}

int main(int argc, char const *argv[])
{
    auto args = parse_arguments(argc, argv);
    auto grammar = std::get<0>(args);
    auto input_string = std::get<1>(args);

    if (!std::get<2>(args))
    {
        return 1;
    }

    /* Call the parser. */
    ThreadPoolParser parser(grammar);
    auto result = parser.parse(input_string);

    // print_result("Results", result);
    // validate_result(result, input_string, grammar);

    return 0;
}