/**
 * Author:
 *   Marco van Eerden
 * Description:
 *   Parses command line arguments.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include "argparse.hpp"

/**
 * @brief Gets the file names for the grammar and input files from the command line
 * arguments.
 *
 * @param argc Amount of arguments.
 * @param argv Array of arguments.
 *
 * @return Tuple with both filenames and a boolean indicating success.
 */
std::tuple<std::string, std::string, bool> get_file_names(int argc, char const *argv[])
{
    switch (argc)
    {
    case 1:
        std::cerr << "Error: missing arguments 'grammar_file' and 'input_file/input_string'" << std::endl;
        return std::make_tuple("", "", false);
    case 2:
        std::cerr << "Error: missing argument 'input_file/input_string'" << std::endl;
        return std::make_tuple("", "", false);
    default:
        return std::make_tuple(std::string(argv[1]), std::string(argv[2]), true);
    }
}

/**
 * @brief Reads the grammar from the grammar file.
 *
 * @param grammar_file Filestream to read the grammar from.
 *
 * @return Grammar read from file.
 */
Grammar get_grammar(std::ifstream& grammar_file)
{
    Grammar grammar;
    std::string token;
    std::vector<std::string> rhs_symbols;

    while (grammar_file)
    {
        std::string line;
        std::string lhs;
        std::vector<std::string> rhs;
        std::getline(grammar_file, line);
        std::stringstream ss(line);

        ss >> lhs;

        if (lhs == "\0")
        {
            break;
        }

        /* Add lhs of rule to nonterminals. */
        grammar.add_nonterminal(lhs);

        /* The first nonterminal is the start symbol. */
        if (!grammar.has_start_symbol)
        {
            grammar.set_start_symbol(lhs);
        }

        /* Add rhs tokens. */
        while (ss >> token)
        {
            rhs.push_back(token);
            rhs_symbols.push_back(token);
        }

        /* Add production rule to the grammar. */
        grammar.add_production_rule(lhs, rhs);
    }

    grammar_file.close();

    /* Add terminal symbols to the grammar. */
    for (auto symbol : rhs_symbols)
    {
        if (!grammar.production_rules.count(symbol) && !grammar.terminals.count(symbol))
        {
            grammar.add_terminal(symbol);
        }
    }

    return grammar;
}

/**
 * @brief Reads the space-separated input from the input file, or the argument
 * itself if the input file cannot be opened.
 *
 * @param input_file Filestream to read the input from.
 * @param argument Command line argument to read the input from.
 *
 * @return Input sequence of symbols.
 */
std::vector<std::string> get_input(std::ifstream& input_file, std::string argument)
{
    std::vector<std::string> input;
    std::string token;

    if (!input_file)
    {
        std::stringstream ss(argument);

        while (ss >> token)
        {
            input.push_back(token);
        }
    }
    else
    {
        while (input_file >> token)
        {
            input.push_back(token);
        }

        input_file.close();
    }

    return input;
}

/**
 * @brief Gets the grammar and input from the command line arguments.
 *
 * @param argc Amount of arguments.
 * @param argv Array of arguments.
 *
 * @return Tuple with grammar, input and a boolean indicating success.
 */
std::tuple<Grammar, std::vector<std::string>, bool> parse_arguments(int argc, char const *argv[])
{
    auto file_names = get_file_names(argc, argv);
    std::string grammar_file_name = std::get<0>(file_names);
    std::string input_file_name = std::get<1>(file_names);
    std::ifstream grammar_file(grammar_file_name);
    std::ifstream input_file(input_file_name);
    Grammar grammar;
    std::vector<std::string> input_string;

    if (!std::get<2>(file_names))
    {
        return std::make_tuple(grammar, input_string, false);
    }

    if (!grammar_file)
    {
        std::cerr << "Error: unable to open file '" << grammar_file_name << "'" << std::endl;
        return std::make_tuple(grammar, input_string, false);
    }

    grammar = get_grammar(grammar_file);
    input_string = get_input(input_file, input_file_name);

    return std::make_tuple(grammar, input_string, true);
}