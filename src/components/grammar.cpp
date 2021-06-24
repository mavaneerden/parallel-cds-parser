/**
 * Author:
 *   Marco van Eerden
 * Description:
 *   Definition of members of the Grammar class from 'grammar.hpp'.
 */

#include <algorithm>
#include <iostream>
#include "grammar.hpp"

/**
 * @brief Creates a grammar with the provided start symbol.
 *
 * @param start Start symbol.
 */
Grammar::Grammar(std::string start)
{
    add_nonterminal(start);
    set_start_symbol(start);
}

/**
 * @brief Adds a terminal to the grammar. Returns ture if the symbol was added
 * successfully, false otherwise.
 *
 * @param symbol std::string to be added.
 *
 * @return A boolean indicating success or failure of the operation.
 */
bool Grammar::add_terminal(std::string symbol)
{
    bool success = !nonterminals.count(symbol);
    success = success && terminals.insert(symbol).second;
    success = success && symbols.insert(symbol).second;

    if (!success)
    {
        std::cerr << "Error: invalid terminal symbol '" << symbol << "'" << std::endl;
    }

    return success;
}

/**
 * @brief Adds a nonterminal to the grammar. Returns ture if the symbol was added
 * successfully, false otherwise.
 *
 * @param symbol std::string to be added.
 *
 * @return A boolean indicating success or failure of the operation.
 */
bool Grammar::add_nonterminal(std::string symbol)
{
    bool success = !terminals.count(symbol);
    success = success && nonterminals.insert(symbol).second;
    success = success && symbols.insert(symbol).second;

    return success;
}

/**
 * @brief Sets the start symbol of the grammar. Returns ture if the start symbol was set
 * successfully, false otherwise.
 *
 * @param symbol Start symbol to be set.
 *
 * @return A boolean indicating success or failure of the operation.
 */
bool Grammar::set_start_symbol(std::string symbol)
{
    bool success = nonterminals.count(symbol);

    if (success)
    {
        start_symbol = symbol;
        has_start_symbol = true;
    }

    return success;
}

/**
 * @brief Adds a production rule to the grammar. Returns true if the rule was added
 * successfully, false otherwise.
 *
 * @param lhs Left-hand side of the production rule.
 * @param rhs Right-hand side of the production rule.
 *
 * @return A boolean indicating success or failure of the operation.
 */
void Grammar::add_production_rule(std::string lhs, std::initializer_list<std::string> rhs)
{
    add_production_rule(lhs, std::vector<std::string>(rhs));
}

/**
 * @brief Adds a production rule to the grammar.
 *
 * @param lhs Left-hand side of the production rule.
 * @param rhs Right-hand side of the production rule.
 *
 */
void Grammar::add_production_rule(std::string lhs, std::vector<std::string> rhs)
{
    production_rules.insert({lhs, rhs});
}

/**
 * @brief Get a vector of production rules according to its left-hand side.
 *
 * @param lhs Left-hand side of the production rules.
 *
 * @return A vector of production rules with left-hand side equal to `lhs`.
 */
std::vector<production_rule_t> Grammar::get_production_rules(std::string lhs)
{
    auto range = production_rules.equal_range(lhs);
    std::vector<production_rule_t> rules;

    std::for_each(
        range.first,
        range.second,
        [&rules](auto elem) {
            rules.push_back(elem);
        }
    );

    return rules;
}