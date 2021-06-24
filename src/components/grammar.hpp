/**
 * Author:
 *   Marco van Eerden
 * Description:
 *   Declares the Grammar class, which represents a context-free grammar.
 */

#pragma once

#include <unordered_map>
#include "../utilities/hash_custom.hpp"
#include "../utilities/types.hpp"

/**
 * Represents a context-free grammar.
 */
class Grammar
{
public:
    /* Set of symbols in the grammar. */
    std::unordered_set<std::string> symbols;
    /* Set of terminals in the grammar. */
    std::unordered_set<std::string> terminals;
    /* Set of nonterminals in the grammar. */
    std::unordered_set<std::string> nonterminals;
    /* Set of production rules. Maps left-hand side to right-hand side. */
    std::unordered_multimap<std::string, std::vector<std::string>> production_rules;
    /* Start symbol of the grammar. */
    std::string start_symbol;
    /* Indicates if start symbol is set. */
    bool has_start_symbol = false;
public:
    Grammar() = default;
    Grammar(std::string start_symbol);
public:
    bool add_terminal(std::string symbol);
    bool add_nonterminal(std::string symbol);
    bool set_start_symbol(std::string symbol);
    void add_production_rule(std::string lhs, std::initializer_list<std::string> rhs);
    void add_production_rule(std::string lhs, std::vector<std::string> rhs);
    std::vector<production_rule_t> get_production_rules(std::string lhs);
};
